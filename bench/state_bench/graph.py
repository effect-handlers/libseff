from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle


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
(ax, ax2) = fig.subplots(2, 1, sharex=True, gridspec_kw={'height_ratios': [1, 2]})

grapher_paramless(res, ax2)
grapher_paramless(list(filter(lambda x: x['measurement'] > 0.5, res)), ax)

# ax.set_yscale('log')
ax.set_ylim(4, 15)
ax2.set_ylim(0, 2)
ax2.legend()

ax.spines['bottom'].set_visible(False)
ax.set_xlabel(None)
ax2.spines['top'].set_visible(False)
ax.xaxis.tick_top()
ax.tick_params(labeltop=False)  # don't put tick labels at the top
# ax2.xaxis.tick_bottom()

d = .015  # how big to make the diagonal lines in axes coordinates
# arguments to pass to plot, just so we don't keep repeating them
kwargs = dict(transform=ax.transAxes, color='k', clip_on=False)
ax.plot((-d, +d), (-d, +d), **kwargs)        # top-left diagonal
ax.plot((1 - d, 1 + d), (-d, +d), **kwargs)  # top-right diagonal

kwargs.update(transform=ax2.transAxes)  # switch to the bottom axes
ax2.plot((-d, +d), (1 - d, 1 + d), **kwargs)  # bottom-left diagonal
ax2.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)  # bottom-right diagonal


# plt.show()
fig.savefig('bench/state_bench/output/state.png', bbox_inches = "tight")