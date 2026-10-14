import numpy as np, warnings
warnings.filterwarnings('ignore')
def bf(u): return float(np.array([u], dtype=np.uint32).view(np.float32)[0])
def ub(f): return int(np.array([f], dtype=np.float32).view(np.uint32)[0])
def mag(u): return u & 0x7fffffff
def isnan(u): return mag(u) > 0x7f800000
def iszero(u): return mag(u) == 0
def flush(u): return u & 0x80000000 if (u & 0x7f800000) == 0 else u
