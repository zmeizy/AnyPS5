import sys, time
from compare import *
def compare_probe(name, limit_modes=None):
    pr = next(x for x in P.PROBES if x['name'] == name)
    summary = []
    for body_name, body in pr['bodies'].items():
        entries = [e for e in R['probes'] if e['probe'] == name and e['body_name'] == body_name and 'results' in e]
        modes = [e['mode'] for e in entries]
        gpu = {i: res for i, (bn, m, rows, res) in enumerate(r for r in runs(name) if r[0] == body_name)}
        rows = pr['rows']()
        t = time.time()
        ours, errors = recompiler(body, pr['outs'], modes, rows)
        for i, m in enumerate(modes):
            bad = [[] for _ in range(pr['outs'])]
            for r, (g, o) in enumerate(zip(gpu[i], ours[i])):
                for k in range(pr['outs']):
                    if g[k] != o[k]: bad[k].append(r)
            summary.append((body_name, {k: m[k] for k in ('ieee','denorm32','denorm16','round16')}, [len(b) for b in bad], bad))
        print(f"{name}/{body_name}: {time.time()-t:.0f}s errors={errors[:3]}", flush=True)
    return summary
if __name__ == "__main__":
    for item in compare_probe(sys.argv[1]):
        print(item[0], item[1], 'mismatches per output:', item[2])
