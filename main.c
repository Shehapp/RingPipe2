#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define NODES 4
#define LOG_INFO(...)fprintf(log_file, __VA_ARGS__);fflush(log_file);

typedef enum
{
    READ_END, //0
    WRITE_END //1
} PIPE_ENDS;

int pipefdSend[2];
int pipefdRecv[2];
int pipefdMaster[2];

FILE *log_file = NULL;

char *msg[] = {"it", "is", "happening", "again!"}; //! //https://www.youtube.com/watch?v=JPdZNCgIDek
#define WORDS_COUNT 4

void swap(int pipe1[2], int pipe2[2]);
void animate_text(size_t msg_id);
void sigint_handler(int sig);

void echoMsg(size_t node_id)
{
    close(pipefdRecv[WRITE_END]);
    close(pipefdSend[READ_END]);
    {
        char msg_id;
        while(read(pipefdRecv[READ_END], &msg_id, 1) > 0)
        {
            LOG_INFO("process %ld recieved \"%s\"\n", node_id, msg[msg_id]);
            animate_text((size_t) msg_id);

            msg_id = (msg_id + 1) % WORDS_COUNT;

            LOG_INFO("process %ld is Sending \"%s\"\n", node_id, msg[msg_id]);
            write(pipefdSend[WRITE_END], &msg_id, 1);
        }
    }
    close(pipefdRecv[READ_END]);
    close(pipefdSend[WRITE_END]);

    wait(NULL);
    _exit(0);
}

void buildRing(int i)
{
    if(i > NODES) return;
    if(i < NODES)
    {
        close(pipefdRecv[READ_END]);
        close(pipefdRecv[WRITE_END]);
        swap(pipefdSend, pipefdRecv);
        assert(pipe(pipefdSend) > -1);
    }
    else if(i == NODES)
    {
        close(pipefdRecv[READ_END]);
        close(pipefdRecv[WRITE_END]);
        swap(pipefdSend, pipefdRecv);
        close(pipefdSend[WRITE_END]);
        swap(pipefdSend, pipefdMaster);
        echoMsg(i);
        return;
    }
    pid_t child_pid = fork();   
    if(child_pid == 0)  //child
    {
        buildRing(i + 1);
        close(pipefdMaster[READ_END]);
        close(pipefdMaster[WRITE_END]);
        echoMsg(i + 1);
    }
}

int main(void)  
{
    log_file = fopen("logs.txt", "w+");
    if(!log_file)
    {
        perror("fopen");
        return 1;
    }

    assert(pipe(pipefdSend) > -1);
    assert(pipe(pipefdMaster) > -1);

    buildRing(1);
    close(pipefdSend[READ_END]);
    close(pipefdMaster[WRITE_END]);

  
    //! start
    char msg_id = 0;
    LOG_INFO("process 1 is sending \"%s\"\n", msg[msg_id]);
    write(pipefdSend[WRITE_END], &msg_id, 1);

    char recv_msg_id;
    while(read(pipefdMaster[READ_END], &recv_msg_id, 1) > 0)
    {
        LOG_INFO("process 1 recieve \"%s\"\n", msg[recv_msg_id]);
        animate_text((size_t) recv_msg_id);

        recv_msg_id = (recv_msg_id + 1) % WORDS_COUNT;

        usleep(1000 * 100);
        LOG_INFO("\n\nprocess 1 is Sending \"%s\"\n", msg[recv_msg_id]);
        write(pipefdSend[WRITE_END], &recv_msg_id, 1);
    }

    close(pipefdSend[WRITE_END]); 
    close(pipefdMaster[READ_END]);
    wait(NULL);
    
    return 0;
}

void swap(int pipe1[2], int pipe2[2])
{
    for(int i = 0; i < 2; i++)
    {
        pipe1[i] ^= pipe2[i];
        pipe2[i] ^= pipe1[i];
        pipe1[i] ^= pipe2[i];
    }
}

void animate_text(size_t msg_id) 
{    
    if(msg_id == 0) printf("\033[2J\033[H");
    int len = strlen(msg[msg_id]);

    for (int i = 1; i <= len; i++) {
        printf("\r"); 
        for (int j = 0; j < i; j++) {
            printf("%c", msg[msg_id][j]);
        }
        fflush(stdout);
        usleep(50000); 
    }

    printf("\n");
    fflush(stdout);
    usleep(200000);
}