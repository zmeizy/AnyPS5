import os, struct, subprocess, sys, tempfile
from pathlib import Path
from load import *
HARNESS = os.environ.get("ANYPS5_PROBE_HARNESS", "build/tests/agc_driver_zz_probe_harness_tests")
def assemble(body, outs):
    lines = ["v_lshlrev_b32 v1, 2, v0", "v_lshlrev_b32 v3, 4, v0"]
    lines += [f"buffer_load_dword v{4+k}, v1, s[0:3], 0 idxen offset:{4*k}" for k in range(4)]
    lines += ["s_waitcnt vmcnt(0)"] + [f"v_mov_b32 v{10+k}, 0" for k in range(16)]
    lines += [body]
    lines += [f"buffer_store_dword v{10+k}, v3, s[4:7], 0 idxen offset:{4*k}" for k in range(outs)]
    lines += ["s_endpgm"]
    with tempfile.TemporaryDirectory() as d:
        Path(d, "k.s").write_text("\n".join(lines) + "\n")
        subprocess.run(["llvm-mc", "-triple=amdgcn", "-mcpu=gfx1013", "-filetype=obj", f"{d}/k.s", "-o", f"{d}/k.o"], check=True)
        subprocess.run(["llvm-objcopy", "-O", "binary", "--only-section=.text", f"{d}/k.o", f"{d}/k.bin"], check=True)
        data = Path(d, "k.bin").read_bytes()
    return list(struct.unpack(f"<{len(data)//4}I", data))
def float_mode(m):
    return (m['round32'] | m['round16'] << 2 | m['denorm32'] << 4 | m['denorm16'] << 6, m['dx10_clamp'], m['ieee'], m['fp16_overflow'])
def recompiler(body, outs, modes, rows):
    code = assemble(body, outs)
    job = [len(code)] + code + [len(modes)] + [v for m in modes for v in float_mode(m)] + [len(rows)] + [v for r in rows for v in r]
    with tempfile.TemporaryDirectory() as d:
        Path(d, "job.bin").write_bytes(struct.pack(f"<{len(job)}I", *job))
        p = subprocess.run([HARNESS, f"{d}/job.bin", f"{d}/out.bin"], capture_output=True, text=True)
        if p.returncode: raise RuntimeError(p.stderr[-2000:])
        errors = sorted(set(line for line in p.stderr.splitlines() if line.startswith("mode")))
        data = Path(d, "out.bin").read_bytes()
    vals = struct.unpack(f"<{len(data)//4}I", data)
    per_mode = len(rows) * 16
    return [[vals[mi*per_mode + i*16: mi*per_mode + i*16 + outs] for i in range(len(rows))] for mi in range(len(modes))], errors
