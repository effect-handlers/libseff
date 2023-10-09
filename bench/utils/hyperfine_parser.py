import json

def parse_hyperfine(f):
    results = json.load(f)['results']

    res = []
    for r in results:
        curr = {}
        curr['cmd'] = r['command']
        curr['unit'] = 'seconds'
        curr['measurement'] = r['mean']

        curr['label'] = r['command']
        curr['stddev'] = r['stddev']
        curr['measurements'] = r['times']
        curr['parameters'] = r['parameters']

        res.append(curr)

    return res

