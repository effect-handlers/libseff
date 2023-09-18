# Building a webserver with libseff
> _Just some notes and resources._
> _Not organized_
## Random concepts

### Async IO
- epoll
- io_uring
- poll, I guess

### Event loop
_Do we want one?_
- libev
- libuv

### HTTP lib
- https://www.libhttp.org/api-reference/
  Prob too much, it seems to handle request handlers and connections

## Resources
- Ocaml paper modern benchmarks
  https://github.com/ocaml-multicore/retro-httpaf-bench/tree/master
- HTTP benchmarking tool wrk2
  https://github.com/giltene/wrk2
- 10x Tokio schedules, maybe
  https://tokio.rs/blog/2019-10-scheduler
- good guide on socket
  https://beej.us/guide/bgnet/

## Ideas

### Explicit effects vs powerrful primitives

- Should we take the approach of a powerful primitive (Suspend) that would allow us to implement other effects (like read, accept, etc) over it? Or maybe smaller primitives

### Our own event loop, vs libev or whatever
- Effects can be used as just a screen over an already implemented event loop, prob not a good idea


### commands
./bench/wrk2/wrk -t 24 -d10s -L -R 100000 -c 1000 http://localhost:8082
./bench/wrk2/wrk -t 24 -d10s -L -R 100000 -c 1000 -s ./bench/http_server_bench/utils/check.lua http://localhost:8082

perf record -F 500 -a -g --call-graph dwarf -- ~/Repos/retro-httpaf-bench/build/rust_hyper.exe
