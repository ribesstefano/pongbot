#ifndef DQNET_DENSE_LAYER_H_
#define DQNET_DENSE_LAYER_H_

#include "dqnet/conv_layer.h"
#include "dqnet/dqnet_utils.h"

typedef enum _DenseArchitecture {
  kFolded, kUnrolled
} DenseArchitecture;

typedef enum _ActivationFunction {
  kLinear, kReLU
} ActivationFunction;

template <int L_i, int L_o, typename TypeIn, typename TypeOut, typename TypeW,
  int U = 8, DenseArchitecture Arch = kUnrolled>
struct DenseParams {
  static const int L_in = L_i;
  static const int L_out = L_o;
  static const int weight_size = L_i * L_o;
  static const int bias_size = L_o;
  static const int size = weight_size + bias_size;
  static const int unroll_factor = U;
  static const DenseArchitecture architecture = Arch;
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
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=DenseP::unroll_factor dim=1
#pragma HLS ARRAY_PARTITION variable=w cyclic factor=DenseP::unroll_factor dim=2

  assert(ConvP::C_out * ConvP::W_out * ConvP::H_out == DenseP::L_in);
  if (DenseP::architecture == kUnrolled) {
    assert(DenseP::unroll_factor > 1); // It will anyway break at compile time.
    using AdderType = typename DenseP::Dout;
    AdderType fm_sum;
    AdderType fm[DenseP::unroll_factor];
    FlattenDense_Unrolled:
    for (int i = 0; i < DenseP::L_out; ++i) {
      for (int wd = 0; wd < ConvP::W_out; ++wd) {
        for (int ht = 0; ht < ConvP::H_out; ++ht) {
          for (int ch = 0; ch < ConvP::C_out / DenseP::unroll_factor; ++ch) {
#pragma HLS PIPELINE II=1
            if (wd == 0 && ht == 0 && ch == 0) {
              fm_sum = 0;
            }
            for (int j = 0; j < DenseP::unroll_factor; ++j) {
              const int ch_idx = ch * DenseP::unroll_factor + j;
              const int w_idx = wd * ConvP::H_out * ConvP::C_out + ht * ConvP::C_out + ch_idx;
              fm[j] = fm_in[ch_idx][wd][ht] * w[i][w_idx];
#pragma HLS RESOURCE variable=fm[j] core=DSP48
            }
            fm_sum += adder::adder_tree<AdderType, DenseP::unroll_factor>(fm);
            if (wd == ConvP::W_out - 1 && ht == ConvP::H_out - 1 && ch == ConvP::C_out / DenseP::unroll_factor - 1) {
              fm_out[i] = ReLU(fm_sum + bias[i]);
              // fm_out[i] = fm_sum + bias[i];
            }
          }
        }
      }
    }
  } else if (DenseP::architecture == kFolded) {
    typename DenseP::Dout fm;
    FlattenDense:
    for (int i = 0; i < DenseP::L_out; ++i) {
      for (int wd = 0; wd < ConvP::W_out; ++wd) {
        for (int ht = 0; ht < ConvP::H_out; ++ht) {
          for (int ch = 0; ch < ConvP::C_out; ++ch) {
#pragma HLS PIPELINE II=1
            if (wd == 0 && ht == 0 && ch == 0) {
              fm = 0;
            }
            const int w_idx = wd * ConvP::H_out * ConvP::C_out + ht * ConvP::C_out + ch;
            fm += fm_in[ch][wd][ht] * w[i][w_idx];
#pragma HLS RESOURCE variable=fm core=DSP48
            if (wd == ConvP::W_out - 1 && ht == ConvP::H_out - 1 && ch == ConvP::C_out - 1) {
              fm_out[i] = ReLU(fm + bias[i]);
              // fm_out[i] = fm + bias[i];
            }
          }
        }
      }
    }
  }
}

template <typename params>
void Dense(const typename params::Din fm_in[params::L_in],
           const typename params::Dw w[params::L_out][params::L_in],
           const typename params::Dw bias[params::L_out],
           typename params::Dout fm_out[params::L_out]) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=params::unroll_factor dim=1
#pragma HLS ARRAY_PARTITION variable=w cyclic factor=params::unroll_factor dim=2
  if (params::architecture == kUnrolled) {
    assert(params::unroll_factor > 1); // It will anyway break at compile time.
    using AdderType = typename params::Dout;
    AdderType fm_sum;
    AdderType fm[params::unroll_factor];
    Dense_Unrolled:
    for (int i = 0; i < params::L_out; ++i) {
      for (int j = 0; j < params::L_in / params::unroll_factor; ++j) {
#pragma HLS PIPELINE II=1
        if (j == 0) {
          fm_sum = 0;
        }
        for (int k = 0; k < params::unroll_factor; ++k) {
          const int u_idx = j * params::unroll_factor + k;
          fm[k] = fm_in[u_idx] * w[i][u_idx];
#pragma HLS RESOURCE variable=fm[k] core=DSP48
        }
        fm_sum += adder::adder_tree<AdderType, params::unroll_factor>(fm);
        if (j == params::L_in / params::unroll_factor - 1) {
          fm_out[i] = fm_sum + bias[i];
        }
      }
    }
  } else if (params::architecture == kFolded) {
    typename params::Dout fm;
    Dense:
    for (int i = 0; i < params::L_out; ++i) {
      for (int j = 0; j < params::L_in; ++j) {
#pragma HLS PIPELINE II=1
        if (j == 0) {
          fm = 0;
        }
        fm += fm_in[j] * w[i][j];
#pragma HLS RESOURCE variable=fm core=DSP48
        if (j == params::L_in - 1) {
          fm_out[i] = fm + bias[i];
        }
      }
    }
  }
}

#endif // end DQNET_DENSE_LAYER_H_