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

big_depth = [0, 10, 20, 40, 60, 80, 100]

fig = plt.figure()
ax = fig.add_subplot(111)

grapher(list(filter(lambda x: (f"{x['label']}{x['parameters']['variation']}" in good_ones) and (int(x['parameters']['depth']) in big_depth), res)), ax, parameter_name='depth')

ax.set_xlabel('Depth')
ax.set_ylabel('Time (seconds)')
# ax.set_ylim(top=15)

fig.savefig('bench/state_paper_bench/output/state_good.png', bbox_inches = "tight")


fig = plt.figure()
ax = fig.add_subplot(111)

grapher(list(filter(lambda x: (f"{x['label']}{x['parameters']['variation']}" in bad_ones) and (int(x['parameters']['depth']) in big_depth), res)), ax, parameter_name='depth')

ax.set_xlabel('Depth')
ax.set_ylabel('Time (seconds)')

fig.savefig('bench/state_paper_bench/output/state_bad.png', bbox_inches = "tight")


fig = plt.figure()
ax = fig.add_subplot(111)

grapher(list(filter(lambda x: (f"{x['label']}{x['parameters']['variation']}" in good_ones) and (float(x['parameters']['depth']) < 10), res)), ax, parameter_name='depth')

ax.set_xlabel('Depth')
ax.set_ylabel('Time (seconds)')

fig.savefig('bench/state_paper_bench/output/state_small.png', bbox_inches = "tight")

res = []
for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)


baseValue = None
for r in res:
    if r['label'] == 'native' and r['parameters']['variation'] == '' and r['parameters']['depth'] == '0':
        baseValue = float(r['measurement'])

# Add general
general = {'Case': 'General'}
optimal = {'Case': 'Optimal'}
for r in res:
    if r['parameters']['depth'] == '0':
        m = float(r['measurement'])
        s = float(r['stddev'])
        l = r['label']
        if l == 'cpp-effects':
            l = 'cppeffects'
        l = '\\' + l
        # d['Mean [ms]'] = f"{(m * 1000):.2f} ± {(s * 1000):.2f}"
        value = f"{(m * 1000):.2f}s ({(m / baseValue):.2f}x)"
        # print(f"{r['label']}{r['parameters']['variation']}")
        if (f"{r['label']}{r['parameters']['variation']}" in good_ones):
            optimal[l] = value
        if (f"{r['label']}{r['parameters']['variation']}" in bad_ones):
            general[l] = value

# for r in res:
#     d = {}
#     d['Framework'] = r['label']
#     d['Depth'] = r['parameters']['depth']
#     m = float(r['measurement'])
#     s = float(r['stddev'])
#     d['Mean [ms]'] = f"{(m * 1000):.2f} ± {(s * 1000):.2f}"
#     d['Relative'] = f"{(m / baseValue):.2f}"
#     table.append(d)

# table = sorted(table, key = lambda x: float(x['Relative']))

with open("bench/state_paper_bench/output/state.tex", "w") as f:
    f.write(format_dict([general, optimal]))