import matplotlib.pyplot as plt


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

markers = iter(["v", "^", "s", "D", "p", "|", "x"])
colours = iter(["blue", "gray", "magenta", "yellow", "green", "orange", "lila"])

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


def grapher(results, ax=plt, parameter_name = None):

    if not parameter_name:
        parameter_name = getParamName(results)

    groups = groupByLabel(results)


    for (label, g) in groups.items():

        zipped = [(r['parameters'][parameter_name], float(r['measurement'])) for r in g]
        zipped = sorted(zipped, key=lambda x: float(x[0]))
        unzipped = list(zip(*zipped))
        style = getStyle(label)
        ax.plot(unzipped[0], unzipped[1], label=label, **style)

    ax.legend()
    ax.set_xlabel(parameter_name)
    ax.set_ylabel(results[0]['unit'])
