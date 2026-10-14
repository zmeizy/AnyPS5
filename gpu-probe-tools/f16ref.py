import math, struct
from fractions import Fraction
import numpy as np
def f16bits(v): return int(np.array([v], dtype=np.float16).view(np.uint16)[0])
def f32val(u): return struct.unpack('<f', struct.pack('<I', u))[0]
def round_f16(x, mode, unbounded=False):
    """x: python float (exact). mode 0 RNE, 1 +inf, 2 -inf, 3 RTZ. Returns (bits, tiny_before, tiny_unbounded)."""
    neg = math.copysign(1.0, x) < 0
    sign = 0x8000 if neg else 0
    if x == 0: return sign, False, False
    if math.isinf(x): return sign | 0x7c00, False, False
    a = Fraction(abs(x))
    m, ex = math.frexp(abs(x)); e = ex - 1
    def rnd(E):
        q = Fraction(2) ** (E - 10)
        n = a / q
        fl = n.numerator // n.denominator; frac = n - fl
        if mode == 0: up = frac > Fraction(1, 2) or (frac == Fraction(1, 2) and fl & 1)
        elif mode == 3: up = False
        elif mode == 1: up = frac > 0 and not neg
        else: up = frac > 0 and neg
        return (fl + (1 if up else 0)) * q
    r = rnd(max(e, -14))
    ru = rnd(e)
    tiny_before = a < Fraction(2) ** -14
    tiny_unb = ru < Fraction(2) ** -14
    if r > 65504:
        if mode == 0 or (mode == 1 and not neg) or (mode == 2 and neg): return sign | 0x7c00, tiny_before, tiny_unb
        return sign | 0x7bff, tiny_before, tiny_unb
    return sign | f16bits(float(r)), tiny_before, tiny_unb
