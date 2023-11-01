from os import listdir
from os.path import isfile, join
import argparse
import matplotlib.pyplot as plt
from ..utils.hyperfine_parser import parse_hyperfine
from ..utils.valgrind_parser import parse_valgrind
from ..utils.wrk2_parser import parse_wrk2
from ..utils.grapher import grapher, grapher_paramless, getStyle

def graph_http_server(labels, conns, threads, rps, parameter_name, file_name):
    res = []

    for l in labels:
        for c in conns:
            for t in threads:
                for r in rps:
                    name = f'./bench/http_server_bench/output/bench-{l}-{c}-{t}-{r}.txt'
                    with open(name) as f:
                        res += parse_wrk2(f)

    fig = plt.figure()
    ax = fig.add_subplot(111)

    grapher(res, ax, parameter_name=parameter_name)

    fig.savefig(f'bench/http_server_bench/output/{file_name}.png', bbox_inches = "tight")



labels = ['libscheff', 'cohttp_eio', 'nethttp_go', 'rust_hyper']

# THREADS	= 1 8 16 32
# CONNECTIONS = 100 1000 5000 10000 50000
# RPS = 35000 50000 100000 200000 400000 800000 1500000 2000000

parser = argparse.ArgumentParser(
                    prog='graph',
                    description='Graph results from HTTP server benchmark')

parser.add_argument('variable', choices=['threads', 'connections', 'rps'])           # positional argument

args = parser.parse_args()

if args.variable == "threads":
    graph_http_server(labels, [1000], [1, 8, 16, 32], [2000000], 'Number of threads', 'http_threads')
elif args.variable == "connections":
    graph_http_server(labels, [100, 1000, 5000, 10000, 50000], [8], [2000000], 'Connections', 'http_connections')
if args.variable == "rps":
    graph_http_server(labels, [1000], [8], [35000, 50000, 100000, 200000, 400000, 800000, 1500000, 2000000], 'Requests per second', 'http_rps')
