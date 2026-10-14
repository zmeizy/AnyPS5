import subprocess, struct, tempfile
from pathlib import Path
from load import *

def gpu_table(probe, body_name):
    table = {}
    for name, mode, rows, res in runs(probe):
        if name != body_name: continue
        key = tuple(sorted(mode.items()))
        table[key] = {tuple(r): o for r, o in zip(rows, res)}
    return table

def lookup(table, mode, row):
    return table[tuple(sorted(mode.items()))][tuple(row)]

def words(asm):
    with tempfile.TemporaryDirectory() as d:
        Path(d, "k.s").write_text(asm)
        subprocess.run(["llvm-mc", "-triple=amdgcn", "-mcpu=gfx1013", "-filetype=obj", f"{d}/k.s", "-o", f"{d}/k.o"], check=True)
        subprocess.run(["llvm-objcopy", "-O", "binary", "--only-section=.text", f"{d}/k.o", f"{d}/k.bin"], check=True)
        data = Path(d, "k.bin").read_bytes()
    return list(struct.unpack(f"<{len(data)//4}I", data))

def program(instruction):
    return words("\n".join([
        "v_lshlrev_b32 v1, 4, v0",
        "v_lshlrev_b32 v2, 2, v0",
        "buffer_load_dwordx4 v[4:7], v1, s[0:3], 0 offen",
        "s_waitcnt vmcnt(0)",
        "v_mov_b32 v10, 0",
        instruction,
        "buffer_store_dword v10, v2, s[4:7], 0 offen",
        "s_endpgm", ""]))

def hexwords(values, per_line=8, indent="    "):
    lines = []
    for i in range(0, len(values), per_line):
        lines.append(indent + ", ".join(f"0x{v:08x}u" for v in values[i:i + per_line]) + ",")
    return "\n".join(lines)
