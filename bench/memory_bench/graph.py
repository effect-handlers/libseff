from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle
res = []
with open('./bench/memory_bench/output/bench-libseff.yaml') as f:
    res += parse_valgrind(f)
with open('./bench/memory_bench/output/bench-cppcoro.yaml') as f:
    res += parse_valgrind(f)
with open('./bench/memory_bench/output/bench-libco.yaml') as f:
    res += parse_valgrind(f)

fig = plt.figure()
ax = fig.add_subplot(111)

grapher(res, ax)
ax.set_ylabel("memory consumed (bytes)")

# plt.show()
fig.savefig('bench/memory_bench/output/memory.png', bbox_inches = "tight")