from os import listdir
from os.path import isfile, join
from glob import glob
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.memory_parser import parse_memory
from ..utils.grapher import grapher, grapher_paramless, getStyle

def graph_http(results, moving_param, filter_params, image_name):
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

    fig.savefig(f'bench/http_server_bench/output/{image_name}.png', bbox_inches = "tight")

reqs = []
for g in glob('./bench/http_server_bench/output/bench-*.txt'):
    with open(g) as f:
        reqs += parse_wrk2(f)

graph_http(reqs, 'Number of threads', {'Connections': '1000', 'Requests per second': '800000'}, 'http_threads')
graph_http(reqs, 'Connections', {'Number of threads': '8', 'Requests per second': '100000'}, 'http_conn')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '1'}, 'http_rps_1')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '8'}, 'http_rps_8')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '16'}, 'http_rps_16')
graph_http(reqs, 'Requests per second', {'Connections': '1000', 'Number of threads': '32'}, 'http_rps_32')


mem = []
for g in glob('./bench/http_server_bench/output/bench-*.memory'):
    with open(g) as f:
        mem += parse_memory(f)

graph_http(mem, 'Number of threads', {'Connections': '1000', 'Requests per second': '100000'}, 'http_memory')