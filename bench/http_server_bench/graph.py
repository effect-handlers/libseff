from os import listdir
from os.path import isfile, join
from glob import glob
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.memory_parser import parse_memory
from ..utils.grapher import grapher, grapher_paramless, getStyle

def graph_http(results, moving_param, filter_params, image_name, xlabel=None, ylabel=None, ytoplim=None):
    fig = plt.figure()
    ax = fig.add_subplot(111)

    filtered_res = []
    for r in results:
        done = True
        for (k, v) in filter_params.items():
            if (r['parameters'][k] != v):
                done = False
                break
        if done:
            filtered_res.append(r)

    grapher(filtered_res, ax, parameter_name=moving_param)
    ax.set_ylim(bottom=0)
    if ytoplim:
        ax.set_ylim(top=ytoplim)
    if xlabel:
        ax.set_xlabel(xlabel)
    if ylabel:
        ax.set_ylabel(ylabel)

    fig.savefig(f'bench/http_server_bench/output/{image_name}.png', bbox_inches = "tight")

reqs = []
for g in glob('./bench/http_server_bench/output/bench-*.txt'):
    with open(g) as f:
        reqs += parse_wrk2(f)

reqs = [r for r in reqs if r['parameters']['Connections'] != '50000']
for r in reqs:
    l = r['label']
    if l == 'libscheff':
        r['label'] = 'libseff'

# graph_http(reqs, 'Number of threads', {'Connections': '100', 'Requests per second': '100000'}, 'http_threads_100')
graph_http(reqs, 'Number of threads', {'Connections': '1000', 'Requests per second': '100000'}, 'http_threads_1000')
graph_http(reqs, 'Number of threads', {'Connections': '5000', 'Requests per second': '100000'}, 'http_threads_5000')
graph_http(reqs, 'Number of threads', {'Connections': '10000', 'Requests per second': '100000'}, 'http_threads_10000')
graph_http(reqs, 'Connections', {'Number of threads': '1', 'Requests per second': '2000000'}, 'http_conn_1')
graph_http(reqs, 'Connections', {'Number of threads': '8', 'Requests per second': '2000000'}, 'http_conn_8')
graph_http(reqs, 'Connections', {'Number of threads': '16', 'Requests per second': '2000000'}, 'http_conn_16')
# graph_http(reqs, 'Connections', {'Number of threads': '32', 'Requests per second': '2000000'}, 'http_conn_32')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '1'}, 'http_rps_1', xlabel='Requests offered per second', ylabel='Requests served per second')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '8'}, 'http_rps_8', xlabel='Requests offered per second', ylabel='Requests served per second')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '16'}, 'http_rps_16', xlabel='Requests offered per second', ylabel='Requests served per second')

graph_http(reqs, 'Requests per second', {'Connections': '10000', 'Number of threads': '1'}, 'http_rps_10000_1', xlabel='Requests offered per second', ylabel='Requests served per second')
graph_http(reqs, 'Requests per second', {'Connections': '10000', 'Number of threads': '8'}, 'http_rps_10000_8', xlabel='Requests offered per second', ylabel='Requests served per second')
graph_http(reqs, 'Requests per second', {'Connections': '10000', 'Number of threads': '16'}, 'http_rps_10000_16', xlabel='Requests offered per second', ylabel='Requests served per second')
# graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '32'}, 'http_rps_32', xlabel='Requests offered per second', ylabel='Requests served per second')

for r in reqs:
    r['measurement'] = r['timeouts'] / r['requests']

# graph_http(reqs, 'Number of threads', {'Connections': '1000', 'Requests per second': '800000'}, 'http_threads_timeouts')
# graph_http(reqs, 'Number of threads', {'Connections': '100', 'Requests per second': '800000'}, 'http_threads_100_timeouts')
graph_http(reqs, 'Number of threads', {'Connections': '1000', 'Requests per second': '1000000'}, 'http_threads_1000_timeouts')
graph_http(reqs, 'Number of threads', {'Connections': '5000', 'Requests per second': '1000000'}, 'http_threads_5000_timeouts')
graph_http(reqs, 'Number of threads', {'Connections': '10000', 'Requests per second': '1000000'}, 'http_threads_10000_timeouts')

graph_http(reqs, 'Connections', {'Number of threads': '1', 'Requests per second': '2000000'}, 'http_conn_1_timeouts')
graph_http(reqs, 'Connections', {'Number of threads': '8', 'Requests per second': '2000000'}, 'http_conn_8_timeouts')
graph_http(reqs, 'Connections', {'Number of threads': '16', 'Requests per second': '2000000'}, 'http_conn_16_timeouts')
# graph_http(reqs, 'Connections', {'Number of threads': '32', 'Requests per second': '2000000'}, 'http_conn_32_timeouts')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '1'}, 'http_rps_1_timeouts')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '8'}, 'http_rps_8_timeouts')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '16'}, 'http_rps_16_timeouts')
# graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '32'}, 'http_rps_32_timeouts')



mem = []
for g in glob('./bench/http_server_bench/output/bench-*.memory'):
    with open(g) as f:
        mem += parse_memory(f)
mem = [m for m in mem if m['parameters']['Connections'] != '50000']
for m in mem:
    l = m['label']
    if l == 'libscheff':
        m['label'] = 'libseff'
    m['measurement'] = m['measurement'] / 1024
    m['unit'] = 'MB'

# graph_http(mem, 'Number of threads', {'Connections': '100', 'Requests per second': '1000000'}, 'http_memory_100', ytoplim=4500)
graph_http(mem, 'Number of threads', {'Connections': '1000', 'Requests per second': '1000000'}, 'http_memory_1000', ytoplim=4500)
graph_http(mem, 'Number of threads', {'Connections': '5000', 'Requests per second': '1000000'}, 'http_memory_5000', ytoplim=4500)
graph_http(mem, 'Number of threads', {'Connections': '10000', 'Requests per second': '1000000'}, 'http_memory_10000', ytoplim=4500)

graph_http(mem, 'Connections', {'Number of threads': '1', 'Requests per second': '1000000'}, 'http_memory_1')
graph_http(mem, 'Connections', {'Number of threads': '8', 'Requests per second': '1000000'}, 'http_memory_8')
graph_http(mem, 'Connections', {'Number of threads': '16', 'Requests per second': '1000000'}, 'http_memory_16')
# graph_http(mem, 'Connections', {'Number of threads': '32', 'Requests per second': '1000000'}, 'http_memory_32')