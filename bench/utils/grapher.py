import matplotlib.pyplot as plt
from itertools import chain, repeat
# print(plt.rcParams) 10
plt.rcParams.update({'font.size': 14})


def getParamName(results):
    name = list(results[0]['parameters'].items())[0][0]

    return name

def groupByLabel(results):
    res = {}
    for r in results:
        lbl = r['label']
        if lbl not in res:
            res[lbl] = []
        res[lbl].append(r)

    return res

styles = {
    'libseff': {
        'color': 'red',
        'marker': 'o',
        'alpha': 1.0,
        'zorder': 10, # so libseff is at the front
        # 'scalex': 'linear'
    },
    'libscheff': {
        'color': 'red',
        'marker': 'o',
        'alpha': 1.0,
        'zorder': 10, # so libseff is at the front
        # 'scalex': 'linear'
    }
}

markers = chain.from_iterable(repeat(["v", "^", "s", "D", "p", "|", "x"]))
colours = chain.from_iterable(repeat(["blue", "gray", "magenta", "purple", "green", "orange"]))


def getStyle(label):
    if label not in styles:
        # create a new one
        newStyle = {
            'color': next(colours),
            'marker': next(markers),
            'alpha': 0.5,
            'zorder': 0,
            # 'scalex': 'linear'
        }
        styles[label] = newStyle

    return styles[label]


def grapher(results, ax=plt, parameter_name = None, extraStyle={}):

    if not parameter_name:
        parameter_name = getParamName(results)

    groups = groupByLabel(results)


    h, l = ax.get_legend_handles_labels()
    labels = set(l)
    for (l, g) in groups.items():

        zipped = []
        for r in g:
            m = float(r['measurement'])
            x = r['parameters'][parameter_name]
            try:
                x = float(x)
            except ValueError:
                pass

            zipped.append((x, m))

        zipped = sorted(zipped, key=lambda x: x[0])
        unzipped = list(zip(*zipped))
        style = getStyle(l)
        ax.plot(unzipped[0], unzipped[1], label=l if l not in labels else None, **{**style, **extraStyle})
        labels.add(l)

    ax.legend()
    ax.set_xlabel(parameter_name)
    ax.set_ylabel(results[0]['unit'])


def grapher_paramless(results, ax=plt):
    zipped = [(r['label'], r['parameters']['variation'], float(r['measurement']), float(r['stddev'])) for r in results]
    zipped = sorted(zipped, key=lambda x: x[2])
    labels = {}
    for (l, p, m, s) in zipped:
        # unzipped = list(zip(*zipped))
        style = getStyle(l)
        ax.errorbar([f"{l}[{p.replace('_', ' ').strip()}]"], [m], [s], fmt='o', label=l if l not in labels else None, linewidth=2, capsize=6, **style)
        labels[l] = 1

    ax.set_xlabel("Benchmarks")
    plt.setp(ax.get_xticklabels(), ha="right", rotation=45)

    ax.set_ylabel(results[0]['unit'])
