# Building a webserver with libseff

## Random concepts

### Async IO
- epoll
- io_uring

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

## Ideas

### Explicit effects vs powerrful primitives

- Should we take the approach of a powerful primitive (Suspend) that would allow us to implement other effects (like read, accept, etc) over it? Or maybe smaller primitives

### Our own event loop, vs libev or whatever
- Effects can be used as just a screen over an already implemented event loop, prob not a good idea


### wrk command
./bench/wrk2/wrk -t 24 -d10s -L -R 50000 -c 1000 http://localhost:8082