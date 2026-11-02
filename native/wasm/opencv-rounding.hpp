// Source fragment installed into pinned OpenCV's intrin_wasm.hpp by build-wasm.py.
// OneOCR: SSE nearest-even, including CVTT-style invalid conversion sentinel.
inline v_int32x4 v_round(const v_float32x4& a)
{
    const v128_t rounded = wasm_f32x4_nearest(a.val);
    const v128_t valid = wasm_v128_and(
        wasm_f32x4_ge(rounded, wasm_f32x4_splat(-2147483648.0f)),
        wasm_f32x4_lt(rounded, wasm_f32x4_splat(2147483648.0f)));
    return v_int32x4(wasm_v128_bitselect(
        wasm_i32x4_trunc_sat_f32x4(rounded),
        wasm_i32x4_splat(-2147483647 - 1), valid));
}
