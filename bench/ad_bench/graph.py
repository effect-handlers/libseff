from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle

res = []

files = [
'./bench/ad_bench/output/bench-naive.json',
'./bench/ad_bench/output/bench-no_alloc.json',
'./bench/ad_bench/output/bench-indirect_prop_t.json',
'./bench/ad_bench/output/bench-ptr_dbl_cast.json',
]

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

fig = plt.figure()
ax = fig.add_subplot(111)
grapher(res, ax)
# ax.set_ylim(0, 0.15)
ax.legend()

# plt.show()
fig.savefig('bench/ad_bench/output/ad.png', bbox_inches = "tight")