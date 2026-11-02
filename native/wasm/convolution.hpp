// OneOCR Wasm numerical profile: native AVX-512 NCHWc accumulation in NCHW storage.
// This source fragment is inserted into pinned MLAS convolve.cpp by build-wasm.py.
#if defined(__EMSCRIPTEN__)
#include <wasm_simd128.h>
#include <cmath>
#include <cfloat>

extern "C" const char* oneocr_wasm_numeric_profile() { return "oneocr-wasm-fma-nchwc16-v2"; }

static inline __attribute__((always_inline)) v128_t OneOcrFma4(v128_t input, float weight, v128_t accumulated) {
    const v128_t high_input=wasm_i32x4_shuffle(input,input,2,3,2,3);
    const v128_t high_acc=wasm_i32x4_shuffle(accumulated,accumulated,2,3,2,3);
    const v128_t weights=wasm_f64x2_splat(double(weight));
    const v128_t low=wasm_f64x2_add(wasm_f64x2_mul(wasm_f64x2_promote_low_f32x4(input),weights),wasm_f64x2_promote_low_f32x4(accumulated));
    const v128_t high=wasm_f64x2_add(wasm_f64x2_mul(wasm_f64x2_promote_low_f32x4(high_input),weights),wasm_f64x2_promote_low_f32x4(high_acc));
    const v128_t mask=wasm_i64x2_splat(0x1fffffff);
    const v128_t halfway=wasm_i64x2_splat(0x10000000);
    v128_t exceptional=wasm_v128_or(wasm_i64x2_eq(wasm_v128_and(low,mask),halfway),wasm_i64x2_eq(wasm_v128_and(high,mask),halfway));
    const v128_t minimum=wasm_f64x2_splat(FLT_MIN);
    const v128_t abs_low=wasm_f64x2_abs(low),abs_high=wasm_f64x2_abs(high);
    const v128_t zero=wasm_f64x2_splat(0);
    exceptional=wasm_v128_or(exceptional,wasm_v128_or(
        wasm_v128_and(wasm_f64x2_lt(abs_low,minimum),wasm_f64x2_gt(abs_low,zero)),
        wasm_v128_and(wasm_f64x2_lt(abs_high,minimum),wasm_f64x2_gt(abs_high,zero))));
    // The normal halfway mask also includes the finite-to-infinity midpoint.
    // Nonfinite operands already have the required IEEE result in f64.
    if(wasm_v128_any_true(exceptional)) {
        // f64 multiply is exact for f32 operands. Only a halfway double
        // rounding, subnormal/overflow or nonfinite result needs full fmaf.
        float a[4],c[4],result[4];wasm_v128_store(a,input);wasm_v128_store(c,accumulated);
        for(int i=0;i<4;i++)result[i]=std::fma(a[i],weight,c[i]);
        return wasm_v128_load(result);
    }
    return wasm_i32x4_shuffle(wasm_f32x4_demote_f64x2_zero(low),wasm_f32x4_demote_f64x2_zero(high),0,1,4,5);
}

static bool OneOcrWasmConvolution(const MLAS_CONV_PARAMETERS* p,const float* input,const float* filter,const float* bias,float* output) {
    if(p->Dimensions!=2)return false;
    const size_t channels=p->InputChannels,filters=p->FilterCount;
    if(!channels || !filters)return false;
    // Mirror NchwcTransformer's alignment eligibility for this fixed profile.
    if(p->GroupCount>1 && !(channels==1 && filters==1) && (channels%16 || filters%16))return false;
    if(p->GroupCount==1 && channels>=16 && channels%4)return false;
    const size_t ih=p->InputShape[0],iw=p->InputShape[1],oh=p->OutputShape[0],ow=p->OutputShape[1];
    const size_t kh=p->KernelShape[0],kw=p->KernelShape[1],kernel=kh*kw;
    const bool pointwise=channels>=16 && kh==1 && kw==1 && !p->Padding[0] && !p->Padding[1] && !p->Padding[2] && !p->Padding[3];
    const size_t block=pointwise?128:channels<16?1:16;
    for(size_t batch=0;batch<p->BatchCount;batch++)for(size_t group=0;group<p->GroupCount;group++) {
        const float* x=input+((batch*p->GroupCount+group)*channels)*ih*iw;
        const float* w=filter+group*filters*channels*kernel;
        float* out=output+((batch*p->GroupCount+group)*filters)*oh*ow;
        for(size_t oc=0;oc<filters;oc++)for(size_t y=0;y<oh;y++)for(size_t start=0;start<ow;start+=4) {
            v128_t total=wasm_f32x4_splat(0);
            if(p->Beta) {
                float previous[4]{};
                for(size_t lane=0;lane<4 && start+lane<ow;lane++)previous[lane]=p->Beta*out[(oc*oh+y)*ow+start+lane];
                total=wasm_v128_load(previous);
            }
            for(size_t begin=0;begin<channels;begin+=block) {
                v128_t partial=wasm_f32x4_splat(0);
                const size_t end=std::min(channels,begin+block);
                // Native first-layer NCHW kernels flush one input channel;
                // blocked kernels flush 16 channels, pointwise kernels 128.
                for(size_t ky=0;ky<kh;ky++)for(size_t kx=0;kx<kw;kx++)for(size_t c=begin;c<end;c++) {
                    const ptrdiff_t iy=ptrdiff_t(y*p->StrideShape[0]+ky*p->DilationShape[0])-ptrdiff_t(p->Padding[0]);
                    const ptrdiff_t ix=ptrdiff_t(start*p->StrideShape[1]+kx*p->DilationShape[1])-ptrdiff_t(p->Padding[1]);
                    v128_t values;
                    if(iy>=0 && iy<ptrdiff_t(ih) && ix>=0 && ix+3<ptrdiff_t(iw) && p->StrideShape[1]==1) {
                        values=wasm_v128_load(x+(c*ih+size_t(iy))*iw+size_t(ix));
                    }else {
                        float pixels[4]{};
                        if(iy>=0 && iy<ptrdiff_t(ih))for(size_t lane=0;lane<4;lane++) {
                            const ptrdiff_t sx=ix+ptrdiff_t(lane*p->StrideShape[1]);
                            if(sx>=0 && sx<ptrdiff_t(iw))pixels[lane]=x[(c*ih+size_t(iy))*iw+size_t(sx)];
                        }
                        values=wasm_v128_load(pixels);
                    }
                    partial=OneOcrFma4(values,w[(oc*channels+c)*kernel+ky*kw+kx],partial);
                }
                total=(begin || p->Beta)?wasm_f32x4_add(partial,total):partial;
            }
            float result[4];wasm_v128_store(result,total);
            for(size_t lane=0;lane<4 && start+lane<ow;lane++) {
                const size_t index=(oc*oh+y)*ow+start+lane;
                out[index]=result[lane];
            }
        }
        MlasActivation(p->Activation,out,bias?bias+group*filters:nullptr,filters,oh*ow,oh*ow);
    }
    return true;
}
#endif
