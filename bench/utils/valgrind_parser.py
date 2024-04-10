import yaml
import os

def parse_valgrind(f):

    # content_lines = f.readlines()

    # This should have name:
    #  bench-<label>.txt
    cmd = os.path.basename(f.name)

    results = yaml.load(f, Loader=yaml.Loader)

    # ==3541637==   total heap usage: 10,001 allocs, 0 frees, 401,440,008 bytes allocated
    res = []
    for (k, v) in results.items():
        curr = {}
        curr['cmd'] = cmd
        curr['unit'] = 'bytes'
        curr['measurement'] = v

        parts = cmd.split(".yaml")[0].split('-')
        curr['label'] = parts[1]
        curr['parameters'] = {
            'padding (bytes)': k
        }

        res.append(curr)

    return res
