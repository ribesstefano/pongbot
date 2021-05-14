#ifndef DQNET_DENSE_LAYER_H_
#define DQNET_DENSE_LAYER_H_

#include "params.h"

template <int L_i, int L_o, typename TypeIn = ActivationType,
  typename TypeOut = ActivationType, typename TypeW = WeightType>
struct DenseParams {
  static const int L_in = L_i;
  static const int L_out = L_o;
  using Din = TypeIn;
  using Dout = TypeOut;
  using Dw = TypeW;
};

template <typename ConvP, typename DenseP>
void FlattenDense(
    const typename ConvP::Din fm_in[ConvP::C_out][ConvP::W_out][ConvP::H_out],
    const typename DenseP::Dw w[DenseP::L_out][DenseP::L_in],
    const typename DenseP::Dw bias[DenseP::L_out],
    typename DenseP::Dout fm_out[DenseP::L_out]) {
  assert(ConvP::C_out * ConvP::W_out * ConvP::H_out == DenseP::L_in);
  for (int i = 0; i < DenseP::L_out; ++i) {
    for (int wd = 0; wd < ConvP::W_out; ++wd) {
      for (int ht = 0; ht < ConvP::H_out; ++ht) {
        for (int ch = 0; ch < ConvP::C_out; ++ch) {
#pragma HLS PIPELINE II=1
          const int w_idx = wd * ConvP::H_out * ConvP::C_out + ht * ConvP::C_out + ch;
          fm_out[i] += fm_in[ch][wd][ht] * w[i][w_idx];
#pragma HLS RESOURCE variable=fm_out[i] core=DSP48
        }
      }
    }
    fm_out[i] += bias[i];
  }
}

template <typename params>
void Dense(const typename params::Din fm_in[params::L_in],
           const typename params::Dw w[params::L_out][params::L_in],
           const typename params::Dw bias[params::L_out],
           typename params::Dout fm_out[params::L_out]) {
  for (int i = 0; i < params::L_out; ++i) {
    for (int j = 0; j < params::L_in; ++j) {
#pragma HLS PIPELINE II=1
      fm_out[i] += fm_in[j] * w[i][j];
#pragma HLS RESOURCE variable=fm_out[i] core=DSP48
    }
    fm_out[i] += bias[i];
  }
}

#endif // end DQNET_DENSE_LAYER_H_