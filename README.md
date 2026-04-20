# RingPipe2

```
// (proc_1) in  ->  pipe1  ->  out  (proc_2)
//   out                               in
//    ^                                 |
//    |                                 V
//   pipe4                            pipe2
//    ^                                 |
//    |                                 V
//    in                               out
// (proc_4) out  <-  pipe3  <-  in  (proc_3)
```
## Run
```console
$ make && ./main
```
