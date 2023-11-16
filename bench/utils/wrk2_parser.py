import yaml
import os
import re


def parse_wrk2(f):
    content = f.read()

    timeoutMatches = re.search("timeout (\d+)", content)
    timeout = 0
    if timeoutMatches:
        timeout = int(timeoutMatches.group(1))

    requestsMatches = re.search("(\d+) requests in", content)
    requests = None
    if requestsMatches:
        requests = int(requestsMatches.group(1))


    content = content.split("JSON Output:\n")[1].replace("\t", "    ")

    res = yaml.load(content, Loader=yaml.Loader)
    # This should have name:
    #  bench-<label>-<conn>-<threads>-<rps>.txt
    cmd = os.path.basename(f.name)

    curr = {}
    curr['cmd'] = cmd
    curr['unit'] = 'requests per second'
    curr['measurement'] = float(res['requests_per_sec'])

    curr['latencies'] = res['latency_distribution']
    curr['timeouts'] = timeout
    curr['requests'] = int(requests)

    parts = cmd.split(".txt")[0].split('-')
    curr['label'] = parts[1]
    curr['parameters'] = {
        "Connections": parts[2],
        "Number of threads": parts[3],
        "Requests per second": parts[4]
    }

    return [curr]
