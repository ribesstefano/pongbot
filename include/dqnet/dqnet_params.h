#ifndef DQNET_DQNET_PARAMS_H_
#define DQNET_DQNET_PARAMS_H_

#include "dqnet/conv_layer.h"
#include "dqnet/dense_layer.h"
#include "dqnet/dqnet_utils.h"

#define IMAGE_W 64
#define IMAGE_H 64
#define IMAGE_C 4
#define CONV_NUM_KERNELS 16
#define CONV_KERNEL_SIZE 4
#define CONV_STRIDE 2

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

template <typename TypeIn = ActivationType, typename TypeOut = ActivationType,
  typename TypeW = WeightType>
struct DQNetParams {
  using Din = TypeIn;
  using Dout = TypeOut;
  using Dw = TypeW;
  typedef ConvParams<32, 8, 4, IMAGE_C, IMAGE_W, IMAGE_H, 0, Din, Dout, Dw, 4> conv1;
  typedef ConvParams<64, 4, 3, conv1::C_out, conv1::W_out, conv1::H_out, 0, Din, Dout, Dw, 1> conv2;
  typedef ConvParams<64, 3, 2, conv2::C_out, conv2::W_out, conv2::H_out, 0, Din, Dout, Dw, 1> conv3;
  typedef DenseParams<conv3::C_out * conv3::W_out * conv3::H_out, 128, Din, Dout, Dw, 8> dense1;
  typedef DenseParams<dense1::L_out, 3, Din, Dout, Dw, 8> dense2;
};

typedef DQNetParams<ActivationType, ActivationType, WeightType> dqnet_param;

#endif // end DQNET_DQNET_PARAMS_H_