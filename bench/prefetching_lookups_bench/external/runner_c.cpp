#include "runner.hpp"

// Can't be bothered to add the Case enum, assuming big
extern "C" int runner_c(long (*testFn)(int [], size_t, int [], size_t, int), int streams, const char * algo_name) {
    auto func = [testFn](State& s) -> long {return testFn(&s.v[0], s.v.size(), &s.lookups[0], s.lookups.size(), s.streams);};
    return runner(func, Case::big, streams, algo_name);
}
