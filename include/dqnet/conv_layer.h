#ifndef DQNET_CONV_LAYER_H_
#define DQNET_CONV_LAYER_H_

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cassert>

#include "dqnet/dqnet_params.h"
#include "dqnet/dqnet_utils.h"

class ConvParameters {
public:
  ConvParameters(const int channels_in, const int row_in, const int col_in,
                 const int kernel_size, const int num_kernels, const int stride,
                 const int padding_size = 0):
                  C_in(channels_in),
                  H_in(row_in),
                  W_in(col_in),
                  K(kernel_size),
                  S(stride),
                  P(padding_size),
                  C_out(num_kernels),
                  H_out(int((row_in - this->K + 2 * this->P) / this->S + 1)),
                  W_out(int((col_in - this->K + 2 * this->P) / this->S + 1)) {}

  ~ConvParameters() {}

  const int K;
  const int S;
  const int P;
  const int C_in;
  const int H_in;
  const int W_in;
  const int C_out;
  const int H_out;
  const int W_out;
};

template <int C_o, int K_i, int S_i, int C_i, int W_i, int H_i, int P_i = 0,
  typename TypeIn = ActivationType, typename TypeOut = ActivationType,
  typename TypeW = WeightType>
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
  using Din = TypeIn;
  using Dout = TypeOut;
  using Dw = TypeW;
};

template <typename params>
void Convolution2D(
    const typename params::Din fm_in[params::C_in][params::W_in][params::H_in],
    const typename params::Dw w[params::C_out][params::C_in][params::K][params::K],
    const typename params::Dw bias[params::C_out],
    typename params::Dout fm_out[params::C_out][params::W_out][params::H_out]) {
#pragma HLS INLINE
  typename params::Dout fm_sum;
  typename params::Dout fm[params::K * params::K];
#pragma HLS ARRAY_PARTITION variable=fm complete dim=1
  Convolution2D:
  for(int row = 0; row < params::H_out; row++) {
    for(int col = 0; col < params::W_out; col++) {
      for(int to = 0; to < params::C_out; to++) {
        for(int ti = 0; ti < params::C_in; ti++) {
#pragma HLS PIPELINE II=1
          if (ti == 0) {
            fm_sum = 0;
            for(int i = 0; i < params::K; i++) {
              for(int j = 0; j < params::K; j++) {
                fm[i * params::K + j] = 0;
              }
            }
          }
          for(int i = 0; i < params::K; i++) {
            for(int j = 0; j < params::K; j++) {
              const int r_idx = params::S * row + i;
              const int c_idx = params::S * col + j;
              fm[i * params::K + j] = w[to][ti][i][j] * fm_in[ti][r_idx][c_idx];
#pragma HLS RESOURCE variable=fm core=DSP48
            }
          }
          fm_sum += adder::adder_tree<typename params::Dout, params::K * params::K>(fm);
          if (ti == params::C_in - 1) {
            fm_out[to][row][col] = fm_sum + bias[to];
#pragma HLS RESOURCE variable=fm_out[to][row][col] core=DSP48
          }
        }
        // fm_out[to][row][col] += bias[to];
      }
    }
  }
}

void conv_gold(const ActivationType *fm_in, const WeightType *weight,
    const WeightType *bias, ActivationType *fm_out);
void conv(const ActivationType *fm_in, const WeightType *weight,
    const WeightType *bias, ActivationType *fm_out);

#endif // end DQNET_CONV_LAYER_H_