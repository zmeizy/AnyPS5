import gzip, json, base64, struct, importlib.util
spec = importlib.util.spec_from_file_location('probes', 'anyps5_gpu_probes.py'); P = importlib.util.module_from_spec(spec); spec.loader.exec_module(P)
import os
R = json.load(gzip.open(os.environ.get('ANYPS5_PROBE_RESULTS', 'results.json.gz'),'rt'))
def runs(probe):
    pr = next(x for x in P.PROBES if x['name'] == probe)
    rows = pr['rows']()
    for e in R['probes']:
        if e['probe'] != probe or 'results' not in e: continue
        data = gzip.decompress(base64.b64decode(e['results']))
        outs = e['outs']
        vals = struct.unpack(f"<{len(rows)*outs}I", data)
        yield e['body_name'], e['mode'], rows, [vals[i*outs:(i+1)*outs] for i in range(len(rows))]
def f32(x): return struct.unpack('<I', struct.pack('<f', x))[0]
