from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.latex_printer import format_dict
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
# ax.set_ylim(0, 0.5)
ax.legend()

# plt.show()
fig.savefig('bench/hot_split_bench/output/hot_split.png', bbox_inches = "tight")



table = []
minValue = min([float(r['measurement']) for r in res])
for r in res:
    m = float(r['measurement'])
    s = float(r['stddev'])
    d = {
        'Case': '\\' + r['label'],
        "Variation": r['parameters']['variation'],
        'Total time (seconds)': f"{(m * 1000):.2f} ± {(s * 1000):.2f}",
        'Cost per call (nanoseconds)': f"{ 1000 * 1000 * 1000 * m / 100000000}",
        'Relative': f"{(m / minValue):.2f} ",
    }

    table.append(d)

with open("bench/hot_split_bench/output/hot_split.tex", "w") as f:
    f.write(format_dict(table))