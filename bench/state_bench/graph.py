from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle
from ..utils.latex_printer import format_table


res = []


files = [
'./bench/state_bench/output/bench-cpp-effects.json',
'./bench/state_bench/output/bench-libseff.json',
'./bench/state_bench/output/bench-native.json',
'./bench/state_bench/output/bench-cppcoro.json',
'./bench/state_bench/output/bench-libhandler.json',
'./bench/state_bench/output/bench-libmpeff.json',
'./bench/state_bench/output/bench-libmprompt.json',
'./bench/state_bench/output/bench-libco.json',
]

for x in files:
    with open(x) as f:
        res += parse_hyperfine(f)

fig = plt.figure()
# ax = fig.add_subplot(111)
(top, middle, bottom) = fig.subplots(3, 1, sharex=True, gridspec_kw={'height_ratios': [1, 1, 1]})

grapher_paramless(res, bottom)
grapher_paramless(res, middle)
grapher_paramless(res, top)

# ax.set_yscale('log')
top.set_ylim(10, 15)
middle.set_ylim(0.5, 2)
bottom.set_ylim(0, 0.25)
middle.legend()


top.spines['bottom'].set_visible(False)
top.set_xlabel(None)

middle.spines['bottom'].set_visible(False)
middle.set_xlabel(None)
middle.spines['top'].set_visible(False)

bottom.spines['top'].set_visible(False)

top.xaxis.tick_top()
top.tick_params(labeltop=False)  # don't put tick labels at the top
middle.tick_params(labeltop=False)  # don't put tick labels at the top
# bottom.tick_params(labeltop=False)  # don't put tick labels at the top
middle.tick_params(labelbottom=False, bottom=False)  # don't put tick labels at the top
# ax2.xaxis.tick_bottom()

d = .015  # how big to make the diagonal lines in axes coordinates
# arguments to pass to plot, just so we don't keep repeating them
kwargs = dict(transform=top.transAxes, color='k', clip_on=False)
top.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
top.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

kwargs = dict(transform=middle.transAxes, color='k', clip_on=False)
middle.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
middle.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

kwargs = dict(transform=middle.transAxes, color='k', clip_on=False)
middle.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # middle-left diagonal
middle.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # middle-right diagonal

kwargs = dict(transform=bottom.transAxes, color='k', clip_on=False)
bottom.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
bottom.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal


# plt.show()
fig.savefig('bench/state_bench/output/state.png', bbox_inches = "tight")

for r in res:
    # So it matches the commands from our paper
    if r['label'] == 'cpp-effects':
        r['label'] = 'cppeffects'
    r['label'] = "\\" + r['label']

with open("bench/state_bench/output/state.tex", "w") as f:
    f.write(format_table(res, base='\\libseff', baseVariation=''))