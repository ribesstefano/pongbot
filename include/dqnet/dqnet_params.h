#ifndef DQNET_DQNET_PARAMS_H_
#define DQNET_DQNET_PARAMS_H_

#define IMAGE_W 64
#define IMAGE_H 64
#define IMAGE_C 4
#define NUM_KERNELS 16
#define KERNEL_SIZE 4
#define STRIDE 2

#ifdef __SYNTHESIS__
#include "ap_int.h"

// typedef ap_int<16> ActivationType;
// typedef ap_int<8> WeightType;

typedef ap_fixed<16, 3> ActivationType;
typedef ap_fixed<8, 3> WeightType;
#else
#include <cstdint>

typedef int16_t ActivationType;
typedef int8_t WeightType;
// typedef float ActivationType;
// typedef float WeightType;
#endif

template<typename T>
inline T min(const T a, const T b) {
#pragma HLS INLINE
  return ((a < b) ? a : b);
}

#endif // end DQNET_DQNET_PARAMS_H_