from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle
from ..utils.latex_printer import format_dict


res = []

files = [
'./bench/state_paper_bench/output/bench-cpp-effects.json',
'./bench/state_paper_bench/output/bench-libseff.json',
'./bench/state_paper_bench/output/bench-native.json',
'./bench/state_paper_bench/output/bench-libhandler.json',
'./bench/state_paper_bench/output/bench-libmpeff.json',
]

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

# for r in res:
#     r['style'] = r['label']
#     r['label'] = f"{r['label']}{r['parameters']['variation']}"



good_ones = [
    'libseff_direct',
    'cpp-effects_optimal',
    'native',
    'libhandler_tail_noop',
    'libmpeff_tail_noop',
]

bad_ones = [
    'libseff',
    'cpp-effects',
    'native',
    'libhandler_general',
    'libmpeff_once',
]


# fig = plt.figure()
# ax = fig.add_subplot(111)

# grapher(res, ax, parameter_name='depth')

# # plt.show()
# fig.savefig('bench/state_paper_bench/output/state.png', bbox_inches = "tight")


fig = plt.figure()
ax = fig.add_subplot(111)

grapher(list(filter(lambda x: f"{x['label']}{x['parameters']['variation']}" in good_ones, res)), ax, parameter_name='depth')

# plt.show()
fig.savefig('bench/state_paper_bench/output/state_good.png', bbox_inches = "tight")


fig = plt.figure()
ax = fig.add_subplot(111)

grapher(list(filter(lambda x: f"{x['label']}{x['parameters']['variation']}" in bad_ones, res)), ax, parameter_name='depth')

# plt.show()
fig.savefig('bench/state_paper_bench/output/state_bad.png', bbox_inches = "tight")

res = []
for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

table = []

baseValue = None
for r in res:
    if r['label'] == 'libseff' and r['parameters']['variation'] == '_direct' and r['parameters']['depth'] == '0':
        baseValue = float(r['measurement'])

for r in res:
    d = {}
    d['Framework'] = r['label']
    d['Depth'] = r['parameters']['depth']
    m = float(r['measurement'])
    s = float(r['stddev'])
    d['Mean [ms]'] = f"{(m * 1000):.2f} ± {(s * 1000):.2f}"
    d['Relative'] = f"{(m / baseValue):.2f}"
    table.append(d)

table = sorted(table, key = lambda x: float(x['Relative']))

with open("bench/state_paper_bench/output/state.tex", "w") as f:
    f.write(format_dict(table))