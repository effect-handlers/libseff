import sys

def format_table(results, baseValue=None):

    header = """% Autogenerated, please don't modify
\\begin{tabular}{ | l | l | r | }
  \\hline
  Library & Variation & Mean [ms] \\\\
  \\hline
"""

    headerBase = """% Autogenerated, please don't modify
\\begin{tabular}{ | l | l | r | r | }
  \\hline
  Library & Variation & Mean [ms] & Relative \\\\
  \\hline
"""


    footer = """  \\hline
\\end{tabular}
"""

    zipped = [(r['label'], r['parameters']['variation'].replace('_', ' ').strip(), float(r['measurement']), float(r['stddev'])) for r in results]
    zipped = sorted(zipped, key=lambda x: x[2])
    res = []
    for (l, p, m, s) in zipped:
        res.append(f"  {l} & {p} & {(m * 1000):.2f} ± {(s * 1000):.2f} " + (f"& {(m / baseValue):.2f} " if baseValue else "") + "\\\\\n")

    return (headerBase if baseValue else header) + ''.join(res) + footer
