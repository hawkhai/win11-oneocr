// MLAS's native FMA3 sigmoid/tanh polynomial ordering, retaining all coefficients.
#include <cmath>
static inline float OneOcrScalarFma(float a,float b,float c) {
#if defined(__EMSCRIPTEN__)
    return std::fma(a,b,c);
#else
    return a*b+c;
#endif
}
#if defined(__EMSCRIPTEN__)
static inline MLAS_FLOAT32X4 OneOcrMlasFma4(MLAS_FLOAT32X4 a,MLAS_FLOAT32X4 b,MLAS_FLOAT32X4 c) {
    float x[4],y[4],z[4],result[4];
    MlasStoreFloat32x4(x,a);MlasStoreFloat32x4(y,b);MlasStoreFloat32x4(z,c);
    for(int i=0;i<4;i++)result[i]=std::fma(x[i],y[i],z[i]);
    return MlasLoadFloat32x4(result);
}
#define MlasMultiplyAddFloat32x4 OneOcrMlasFma4
#endif
