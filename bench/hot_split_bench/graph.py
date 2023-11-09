from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle

res = []

files = [
'./bench/hot_split_bench/output/bench-libseff.json',
'./bench/hot_split_bench/output/bench-native.json',
]

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

fig = plt.figure()
ax = fig.add_subplot(111)
grapher_paramless(res, ax)
ax.set_ylim(0, 0.5)
ax.legend()

# plt.show()
fig.savefig('bench/hot_split_bench/output/hot_split.png', bbox_inches = "tight")