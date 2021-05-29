#ifndef DQNET_CONV_LAYER_H_
#define DQNET_CONV_LAYER_H_

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cassert>

#include "dqnet/dqnet_utils.h"

template <int C_o, int K_i, int S_i, int C_i, int W_i, int H_i, int P_i,
  typename TypeIn, typename TypeOut, typename TypeW, int U = 1>
struct ConvParams {
  static const int C_out = C_o;
  static const int C_in = C_i;
  static const int W_in = W_i;
  static const int H_in = H_i;
  static const int K = K_i;
  static const int S = S_i;
  static const int P = P_i;
  static const int H_out = (int((H_i - K + 2 * P) / S + 1));
  static const int W_out = (int((W_i - K + 2 * P) / S + 1));
  static const int weight_size = C_out * C_in * W_in * H_in * K * K;
  static const int bias_size = C_out;
  static const int size = weight_size + bias_size;
  static const int in_size = C_in * W_in * H_in;
  static const int out_size = C_out * H_out * W_out;
  static const int unroll_factor = U;
  using Din = TypeIn;
  using Dout = TypeOut;
  using Dw = TypeW;
};

template <typename params>
void Convolution2D_Reference(
    const typename params::Din fm_in[params::C_in][params::W_in][params::H_in],
    const typename params::Dw w[params::C_out][params::C_in][params::K][params::K],
    const typename params::Dw bias[params::C_out],
    typename params::Dout fm_out[params::C_out][params::W_out][params::H_out]) {
  typename params::Dout fm_tmp;
  Convolution2D:
  for(int row = 0; row < params::H_out; row++) {
    for(int col = 0; col < params::W_out; col++) {
      for(int to = 0; to < params::C_out; to++) {
        fm_tmp = 0;
        for(int ti = 0; ti < params::C_in; ti++) {
          for(int i = 0; i < params::K; i++) {
            for(int j = 0; j < params::K; j++) {
              const int r_idx = params::S * row + i;
              const int c_idx = params::S * col + j;
              fm_tmp += w[to][ti][i][j] * fm_in[ti][r_idx][c_idx];
            }
          }
        }
        fm_out[to][row][col] = ReLU(fm_tmp + bias[to]);
      }
    }
  }
}

template <typename params>
void Convolution2D(
    const typename params::Din fm_in[params::C_in][params::W_in][params::H_in],
    const typename params::Dw w[params::C_out][params::C_in][params::K][params::K],
    const typename params::Dw bias[params::C_out],
    typename params::Dout fm_out[params::C_out][params::W_out][params::H_out]) {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=params::K dim=2
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=params::K dim=3
#pragma HLS ARRAY_PARTITION variable=w cyclic factor=params::unroll_factor dim=1
#pragma HLS ARRAY_PARTITION variable=w complete dim=3
#pragma HLS ARRAY_PARTITION variable=w complete dim=4
#pragma HLS ARRAY_PARTITION variable=bias cyclic factor=params::unroll_factor dim=1
#pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=params::unroll_factor dim=1
  using adder_t = typename params::Dout;
  Convolution2D:
  for(int row = 0; row < params::H_out; row++) {
    for(int col = 0; col < params::W_out; col++) {
      for(int to = 0; to < params::C_out; to++) {
#pragma HLS UNROLL factor=params::unroll_factor
        typename params::Dout fm_sum;
        typename params::Dout fm[params::K * params::K];
#pragma HLS ARRAY_PARTITION variable=fm complete dim=1
        for(int ti = 0; ti < params::C_in; ti++) {
#pragma HLS PIPELINE II=1
          if (ti == 0) {
            fm_sum = 0;
          }
          for(int i = 0; i < params::K; i++) {
            for(int j = 0; j < params::K; j++) {
              const int r_idx = params::S * row + i;
              const int c_idx = params::S * col + j;
              fm[i * params::K + j] = w[to][ti][i][j] * fm_in[ti][r_idx][c_idx];
#pragma HLS RESOURCE variable=fm[i*params::K+j] core=DSP48
            }
          }
          fm_sum += adder::adder_tree<adder_t, params::K * params::K>(fm);
          if (ti == params::C_in - 1) {
            fm_out[to][row][col] = ReLU(fm_sum + bias[to]);
#pragma HLS RESOURCE variable=fm_out[to][row][col] core=DSP48
          }
        }
      }
    }
  }
}

#endif // end DQNET_CONV_LAYER_H_