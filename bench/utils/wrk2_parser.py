import json
import os

def parse_wrk2(f):
    content = f.read()
    content.split("JSON Output:\n")

    res = json.loads(content)
    # This should have name:
    #  <label>-<param name>-<param value>.txt
    cmd = os.path.basename(f.name)

    curr = {}
    curr['cmd'] = cmd
    curr['unit'] = 'requests per second'
    curr['measurement'] = res['requests_per_sec']

    curr['latencies'] = res['latency_distribution']

    parts = cmd.split(".txt")[0].split('-')
    curr['label'] = parts[0]
    curr['parameters'] = {
        parts[1]: parts[2]
    }

    return [curr]
