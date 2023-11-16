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

values = {}

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

for r in res:
    l = f"{r['label']} {r['parameters']['variation'].replace('_', ' ')}"
    r['label'] = l
    if (l not in values):
        values[l] = {}
    values[l][int(r['parameters']['mults'])] = r['measurement']



fig = plt.figure()
ax = fig.add_subplot(111)
grapher(res, ax, parameter_name='mults')
# ax.set_ylim(0, 0.5)
ax.set_xlabel("Multiplications")
ax.set_ylabel("Time (seconds)")
ax.legend()

# plt.show()
fig.savefig('bench/hot_split_bench/output/hot_split.png', bbox_inches = "tight")


table = []
for (k, d) in values.items():
    res = {
        'Case': k
    }
    for k in [0, 5, 10, 15, 20]:
        minValue = min([values[l][k] for l in values.keys()])
        res[str(k)] = f"{d[k] / minValue :.2f}"
    table.append(res)
# minValue = min([float(r['measurement']) for r in res])
# for r in res:
#     m = float(r['measurement'])
#     s = float(r['stddev'])
#     d = {
#         'Case': '\\' + r['label'],
#         "Variation": r['parameters']['variation'],
#         'Total time (seconds)': f"{(m * 1000):.2f} ± {(s * 1000):.2f}",
#         'Cost per call (nanoseconds)': f"{ 1000 * 1000 * 1000 * m / 100000000}",
#         'Relative': f"{(m / minValue):.2f} ",
#     }

#     table.append(d)

with open("bench/hot_split_bench/output/hot_split.tex", "w") as f:
    f.write(format_dict(table))