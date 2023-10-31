from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle

res = []
with open('./bench/context_switch_bench/output/bench-libseff.json') as f:
    res += parse_hyperfine(f)
with open('./bench/context_switch_bench/output/bench-cppcoro.json') as f:
    res += parse_hyperfine(f)
with open('./bench/context_switch_bench/output/bench-libco.json') as f:
    res += parse_hyperfine(f)

fig = plt.figure()
ax = fig.add_subplot(111)

grapher(res, ax)

# plt.show()
fig.savefig('bench/context_switch_bench/output/context_switch.png', bbox_inches = "tight")