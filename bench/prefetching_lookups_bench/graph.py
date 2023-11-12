from os import listdir
from os.path import isfile, join
import argparse
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle

res = []

files = [
    './bench/prefetching_lookups_bench/output/bench-naive.json',
    # './bench/prefetching_lookups_bench/output/bench-sm.json',
    './bench/prefetching_lookups_bench/output/bench-coro.json',
    # './bench/prefetching_lookups_bench/output/bench-libseff.json',
    './bench/prefetching_lookups_bench/output/bench-libseff_effect_coro.json',
    # './bench/prefetching_lookups_bench/output/bench-libseff_effect_naive.json',
]

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

fig = plt.figure()
ax = fig.add_subplot(111)

grapher(res, ax)

# plt.show()
# fig.savefig('bench/prefetching_lookups_bench/output/prefetch_all.png', bbox_inches = "tight")

# Let's take only the best ones now

best = {}

for r in res:
    l = r['label']
    m = r['measurement']

    if l not in best:
        best[l] = (m, r)
    elif m < best[l][0]:
        best[l] = (m, r)

resBest = map(lambda x: x[1], best.values())
resBest = list(sorted(resBest, key=lambda x: float(x['parameters']['streams'])))

# fig = plt.figure()
# ax = fig.add_subplot(111)

grapher(resBest, ax, extraStyle={'markersize': 12, 'alpha': 1.0})

fig.savefig('bench/prefetching_lookups_bench/output/prefetch.png', bbox_inches = "tight")

# plt.show()
# fig.savefig('bench/prefetching_lookups_bench/output/prefetch_best.png', bbox_inches = "tight")