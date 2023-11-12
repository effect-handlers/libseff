import yaml
import os

def parse_memory(f):
    # VmPeak:	 2164876 kB
    content = f.read().replace("\t", "    ")

    res = yaml.load(content, Loader=yaml.Loader)
    # This should have name:
    #  bench-<label>-<conn>-<threads>-<rps>.txt
    cmd = os.path.basename(f.name)

    curr = {}
    curr['cmd'] = cmd
    curr['unit'] = 'KB'
    curr['measurement'] = float(res['VmPeak'].split(' ')[0])

    parts = cmd.split(".memory")[0].split('-')
    curr['label'] = parts[1]
    curr['parameters'] = {
        "Connections": parts[2],
        "Number of threads": parts[3],
        "Requests per second": parts[4]
    }

    return [curr]
