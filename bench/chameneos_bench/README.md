The Chameneos benchmark[1] is a standard concurrency benchmark that shows a concurrency framework's
performance under a bottleneck. And, when testing a particular shceduling system it also tests the fairness
under a cooperative scenario between many independent processes.

The explanation here[2] is a bit more clear.
Some implementations:
- https://github.com/ocaml-multicore/effects-examples/blob/master/mvar/chameneos.ml (and some other on the directory), from [3]
- https://github.com/rafaelcaue/Benchmarks--AGERE-13/blob/master/chameneos/akka/src/main/scala/chameneos.scala (using Akka, others on the same repo), from [2]


The basic idea (from [2]) is:
- create C differently coloured (blue, red, or yellow), differently named, chameneos creatures;
- each creature will repeatedly go to the meeting place and
meet, or wait to meet, another creature;
- both creatures will change colours to the complement of
the colour of the chameneos that they just met;
- write all of the complements between the three colours
(blue, red, and yellow);
- write the initial colours of all chameneos creatures;
- after N meetings, for each creature write the number of
creatures it met and spell out the number of times that it
met a creature with the same name (should be zero);
- the program halts after writing the sum of the number of
meetings that each creature had (should be 2 ∗ N).


## How to implement

### Effect Handlers
Just define `Meet` as an effect, when it returns it should have either a stop command, or
another chameneos ready to interact with.

Note that this gives us a choice on how to actually implement the handlers, and the options below
are valid (actors, fork/mvar, etc)

### Actors

### Concurrency primitives (fork)

[1]: https://ieeexplore.ieee.org/document/1227495
[2]: https://dl.acm.org/doi/abs/10.1145/2541329.2541339
[3]: https://dl.acm.org/doi/10.1145/3453483.3454039