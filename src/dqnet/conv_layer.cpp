#include "dqnet/conv_layer.h"

WeightType conv1_w[32][4][3][3];
WeightType conv2_w[32][32][3][3];
WeightType conv3_w[32][32][3][3];
WeightType dense1_w[128][1568];
WeightType dense2_w[3][128];

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

template <typename Din, typename Dout, typename Dw>
void Convolution2D(ConvParameters &params, const Din *fm_in,
                   const Dw *weight, const Dw *bias, Dout *fm_out) {
  for(int row = 0; row < params.H_out; row++) {
    for(int col = 0; col < params.W_out; col++) {
      for(int to = 0; to < params.C_out; to++) {
        for(int ti = 0; ti < params.C_in; ti++) {
#pragma HLS PIPELINE II=1
          for(int i = 0; i < params.K; i++) {
            for(int j = 0; j < params.K; j++) {
              const int out_idx = to * params.H_out * params.W_out + row * params.W_out + col;
              const int in_idx = ti * params.H_in * params.W_in + (params.S * row + i) * params.W_in + params.S * col + j;
              const int w_idx = to * params.C_in * params.K * params.K + ti * params.K * params.K + i * params.K + j;
              fm_out[out_idx] += weight[w_idx] * fm_in[in_idx];
            }
          }
        }
      }
    }
  }
}

template <typename Din, typename Dout, typename Dw>
void Convolution2D_Reference(const int C_in, const int R_in, const int channels_in,
                   const int kernel_size, const int padding_size,
                   const int stride, const int num_kernels, const Din *fm_in,
                   const Dw *weight, const Dw *bias, Dout *fm_out) {
  const int K = kernel_size;
  const int S = stride;
  const int P = padding_size;
  const int R = int((R_in - K + 2 * P) / S + 1);
  const int C = int((C_in - K + 2 * P) / S + 1);
  const int M = num_kernels;
  const int N = channels_in;

  for(int row = 0; row < R; row++) { // R: output rows
    for(int col = 0; col < C; col++) { // C: output columns
      for(int to = 0; to < M; to++) { // M: output channels / Number of filters
        for(int ti = 0; ti < N; ti++) { // N: input channels
#pragma HLS PIPELINE II=1
          for(int i = 0; i < K; i++) {
            for(int j = 0; j < K; j++) {
              const int out_idx = to * R * C + row * C + col;
              const int in_idx = ti * R_in * C_in + (S * row + i) * C_in + S * col + j;
              const int w_idx = to * N * K * K + ti * K * K + i * K + j;
              if (S * row + i > R_in || S * col + j > C_in) {
                std::cout << "\t\tout o bounds" << "\n";
              }
              fm_out[out_idx] += weight[w_idx] * fm_in[in_idx];
            }
          }
        }
      }
    }
  }
}

template <typename T>
T floor(const T x) {
  return x - (x % 1);
}

template <
  typename Din,
  typename Dout,
  typename Dw,
  int Tr,
  int Tc,
  int Tm,
  int Tn,
  int K>
void Convolution2D(const int C_in, const int R_in, const int channels_in,
                   const int kernel_size, const int padding_size,
                   const int stride, const int num_kernels, const Din *fm_in,
                   const Dw *weight, const Dw *bias, Dout *fm_out) {
  const int S = stride;
  const int P = padding_size;
  const int R = floor((R_in - K + 2 * P) / S + 1);
  const int C = floor((C_in - K + 2 * P) / S + 1);
  const int M = num_kernels;
  const int N = channels_in;
  assert(Tr <= R_in);
  assert(Tc <= C_in);
  assert(Tm <= M);
  assert(Tn <= N);

  Dout out_buf[2][Tm][Tr][Tc];
  Din in_buf[2][Tn][Tr][Tc][K][K];
  Dw w_buf[2][Tm][Tn][K][K];
#pragma HLS ARRAY_PARTITION variable=out_buf complete dim=0
#pragma HLS ARRAY_PARTITION variable=in_buf complete dim=0
#pragma HLS ARRAY_PARTITION variable=w_buf complete dim=0

  for(int trr = 0; trr < Tr; ++trr) {
    for(int tcc = 0; tcc < Tc; ++tcc) {
      for(int too = 0; too < Tm; ++too) {
        for (int i = 0; i < 2; ++i) {
#pragma HLS PIPELINE II=1
          out_buf[i][too][trr][tcc] = 0;
        }
      }
    }
  }

  int rd_ptr = 0;
  int wr_ptr = 0;

  int row = 0;
  int col = 0;
  int to = 0;
  int ti = 0;
  auto load_buffers = [&]() {
    int ti_offset = Tn;
    int to_offset = 0;
    int tc_offset = 0;
    int tr_offset = 0;
    if (ti + ti_offset >= N) {
      ti_offset = 0;
      to_offset = Tm;
    }
    if (to + to_offset >= M) {
      to_offset = 0;
      tc_offset = Tc;
    }
    if (col + tc_offset >= C) {
      tc_offset = 0;
      tr_offset = Tr;
    }
    for(int trr = 0; trr < Tr; ++trr) {
      for(int tcc = 0; tcc < Tc; ++tcc) {
        for(int too = 0; too < Tm; ++too) {
          for(int tii = 0; tii < Tn; ++tii) {
            for(int i = 0; i < K; ++i) {
              for(int j = 0; j < K; ++j) {
#pragma HLS PIPELINE II=1
                // NOTE: Adding the tile to EVERY index is simply wrong. Find a
                // way to add the offset ONLY to the NEXT one.
                const int tr_idx = row + trr + tr_offset;
                const int tc_idx = col + tcc + tc_offset;
                const int to_idx = to + too + to_offset;
                const int ti_idx = ti + tii + ti_offset;
                const int in_idx = ti_idx * R_in * C_in + (S * tr_idx + i) * C_in + S * tc_idx + j;
                const int w_idx = to_idx * N * K * K + ti_idx * K * K + i * K + j;
                if (S * tr_idx + (K-1) > R_in || S * tc_idx + (K-1) > C_in) {
                  in_buf[rd_ptr][tii][trr][tcc][i][j] = 0;
                } else {
                  in_buf[rd_ptr][tii][trr][tcc][i][j] = fm_in[in_idx];
                }
                w_buf[rd_ptr][too][tii][i][j] = weight[w_idx];
              }
            }
          }
        }
      }
    }
  };

  auto store_buffers = [&]() {
    for(int too = 0; too < Tm; ++too) {
      for(int trr = 0; trr < Tr; ++trr) {
        for(int tcc = 0; tcc < Tc; ++tcc) {
#pragma HLS PIPELINE II=1
          const int to_idx = to + too;
          const int tr_idx = row + trr;
          const int tc_idx = col + tcc;
          if (to_idx < M && tr_idx < R && tc_idx < C) {
            const int out_idx = to_idx * R * C + tr_idx * C + tc_idx;
            fm_out[out_idx] = out_buf[wr_ptr][too][trr][tcc];
            out_buf[wr_ptr][too][trr][tcc] = 0;
          }
        }
      }
    }
  };

  auto flip = [&](int &ptr) {ptr = (ptr == 1) ? 0 : 1;};
  load_buffers(); // loads into 0
  // flip(wr_ptr);
  bool first_cycle = true;
  bool last_cycle = false;
  for(row = 0; row < R; row += Tr) { // R: output rows
    for(col = 0; col < C; col += Tc) { // C: output columns
      for(to = 0; to < M; to += Tm) { // M: output channels / Number of filters
        for(ti = 0; ti < N; ti += Tn) { // N: input channels
          // load weight
          // load input feature map

          flip(rd_ptr);
          if (!last_cycle) {
            load_buffers();
          }
          for(int i = 0; i < K; ++i) {
            for(int j = 0; j < K; ++j) {
#pragma HLS PIPELINE II=1
              for(int trr = 0; trr < Tr; ++trr) {
                for(int tcc = 0; tcc < Tc; ++tcc) {
                  for(int too = 0; too < Tm; ++too) {
                    for(int tii = 0; tii < Tn; ++tii) {
                      out_buf[wr_ptr][too][trr][tcc] += w_buf[wr_ptr][too][tii][i][j] * in_buf[wr_ptr][tii][trr][tcc][i][j];
                    }
                  }
                }
              }
            }
          }
        }
        flip(wr_ptr);
        store_buffers();

        // flip(wr_ptr);
        // if (!first_cycle) {
        //   // store output feature maps
        //   store_buffers();
        //   first_cycle = false;
        // }
        
        if (row + Tr > R && col + Tc > C && to + 2 * Tm > M) {
          last_cycle = true;
        }
      }
    }
  }
  // row -= Tr;
  // col -= Tc;
  // to -= Tm;
  // flip(wr_ptr);
  // store_buffers();
}

void conv(const ActivationType *fm_in, const WeightType *weight,
    const WeightType *bias, ActivationType *fm_out) {
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE m_axi port=fm_in offset=slave depth=1 bundle=dmem_fm_in
#pragma HLS INTERFACE m_axi port=weight offset=slave depth=1 bundle=dmem_weight
#pragma HLS INTERFACE m_axi port=bias offset=slave depth=1 bundle=dmem_bias
#pragma HLS INTERFACE m_axi port=fm_out offset=slave depth=1 bundle=dmem_fm_out
  const int C_in = IMAGE_W;
  const int R_in = IMAGE_H;
  const int channels_in = IMAGE_C;
  const int kernel_size = KERNEL_SIZE;
  const int K = KERNEL_SIZE;
  const int padding_size = 0;
  const int stride = STRIDE;
  const int num_kernels = NUM_KERNELS;
  Convolution2D<ActivationType, ActivationType, WeightType, 8, 8, 4, 1, K>(C_in,
    R_in, channels_in, kernel_size, padding_size, stride, num_kernels, fm_in,
    weight, bias, fm_out);
}

void conv_gold(const ActivationType *fm_in, const WeightType *weight,
    const WeightType *bias, ActivationType *fm_out) {
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE m_axi port=fm_in offset=slave depth=1 bundle=dmem_fm_in
#pragma HLS INTERFACE m_axi port=weight offset=slave depth=1 bundle=dmem_weight
#pragma HLS INTERFACE m_axi port=bias offset=slave depth=1 bundle=dmem_bias
#pragma HLS INTERFACE m_axi port=fm_out offset=slave depth=1 bundle=dmem_fm_out
  const int C_in = IMAGE_W;
  const int R_in = IMAGE_H;
  const int channels_in = IMAGE_C;
  const int kernel_size = KERNEL_SIZE;
  const int padding_size = 0;
  const int stride = STRIDE;
  const int num_kernels = NUM_KERNELS;
  Convolution2D_Reference<ActivationType, ActivationType, WeightType>(C_in,
    R_in, channels_in, kernel_size, padding_size, stride, num_kernels, fm_in,
    weight, bias, fm_out);
}

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
    const typename params::Dw *bias,
    typename params::Dout fm_out[params::C_out][params::W_out][params::H_out]) {
  for(int row = 0; row < params::H_out; row++) {
    for(int col = 0; col < params::W_out; col++) {
      for(int to = 0; to < params::C_out; to++) {
        for(int ti = 0; ti < params::C_in; ti++) {
#pragma HLS PIPELINE II=1
          for(int i = 0; i < params::K; i++) {
            for(int j = 0; j < params::K; j++) {
              const int r_idx = params::S * row + i;
              const int c_idx = params::S * col + j;
              fm_out[to][row][col] += w[to][ti][i][j] * fm_in[ti][r_idx][c_idx];
#pragma HLS RESOURCE variable=fm_out[to][row][col] core=DSP48
            }
          }
        }
      }
    }
  }
}

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

template <typename params>
void init_1d_buffer(const typename params::Dout x,
    typename params::Dout y[params::L_out]) {
#pragma HLS INLINE
  for (int i = 0; i < params::L_out; ++i) {
#pragma HLS PIPELINE II=1
    y[i] = x;
  }
}

template <typename params>
void init_3d_buffer(const typename params::Dout x,
    typename params::Dout y[params::C_out][params::W_out][params::H_out]) {
#pragma HLS INLINE
  for (int i = 0; i < params::C_out; ++i) {
    for (int j = 0; j < params::W_out; ++j) {
      for (int k = 0; k < params::H_out; ++k) {
#pragma HLS PIPELINE II=1
        y[i][j][k] = x;
      }
    }
  }
}

void conv_net(const ActivationType *fm_in, int &action) {
// #pragma HLS INTERFACE s_axilite port=return bundle=ctrl
// #pragma HLS INTERFACE m_axi port=fm_in offset=slave depth=1 bundle=dmem_fm_in
// #pragma HLS INTERFACE s_axilite port=action bundle=ctrl
#pragma HLS INTERFACE ap_fifo port=fm_in
#pragma HLS DATAFLOW

  typedef ConvParams<32, 3, 2, IMAGE_C, IMAGE_W, IMAGE_H> conv1;
  typedef ConvParams<32, 3, 2, conv1::C_out, conv1::W_out, conv1::H_out> conv2;
  typedef ConvParams<32, 3, 2, conv2::C_out, conv2::W_out, conv2::H_out> conv3;
  typedef DenseParams<conv3::C_out * conv3::W_out * conv3::H_out, 128> dense1;
  typedef DenseParams<dense1::L_out, 3> dense2;

  // WeightType conv1_w[conv1::C_out][conv1::C_in][conv1::K][conv1::K];
  // WeightType conv2_w[conv2::C_out][conv2::C_in][conv2::K][conv2::K];
  // WeightType conv3_w[conv3::C_out][conv3::C_in][conv3::K][conv3::K];
  // WeightType dense1_w[dense1::L_out][dense1::L_in];
  // WeightType dense2_w[dense2::L_out][dense2::L_in];
  // std::cout << "conv1_w[" << conv1::C_out << "][" << conv1::C_in << "][" << conv1::K << "][" << conv1::K << "]" << std::endl;
  // std::cout << "conv2_w[" << conv2::C_out << "][" << conv2::C_in << "][" << conv2::K << "][" << conv2::K << "]" << std::endl;
  // std::cout << "conv3_w[" << conv3::C_out << "][" << conv3::C_in << "][" << conv3::K << "][" << conv3::K << "]" << std::endl;
  // std::cout << "dense1_w[" << dense1::L_out << "][" << dense1::L_in << "]" << std::endl;
  // std::cout << "dense2_w[" << dense2::L_out << "][" << dense2::L_in << "]" << std::endl;

  WeightType conv1_b[1];
  WeightType conv2_b[1];
  WeightType conv3_b[1];
  WeightType dense1_b[dense1::L_out];
  WeightType dense2_b[dense2::L_out];

  ActivationType fmi[conv1::C_in][conv1::W_in][conv1::H_in];
  ActivationType fm1[conv1::C_out][conv1::W_out][conv1::H_out];
  ActivationType fm2[conv2::C_out][conv2::W_out][conv2::H_out];
  ActivationType fm3[conv3::C_out][conv3::W_out][conv3::H_out];
  ActivationType fm4[dense1::L_out];
  ActivationType fm5[dense2::L_out];
#pragma HLS ARRAY_PARTITION variable=fmi complete dim=1
#pragma HLS ARRAY_PARTITION variable=fm1 complete dim=1
#pragma HLS ARRAY_PARTITION variable=fm2 complete dim=1
#pragma HLS ARRAY_PARTITION variable=fm3 complete dim=1
#ifndef __SYNTHESIS__
  init_3d_buffer<conv1>(0, fm1);
  init_3d_buffer<conv2>(0, fm2);
  init_3d_buffer<conv3>(0, fm3);
  init_1d_buffer<dense1>(0, fm4);
  init_1d_buffer<dense2>(0, fm5);
#endif

#ifndef __VITIS_HLS__
  auto dma_in = [&]() {
#pragma HLS INLINE
#endif
    InDMA:
    for (int i = 0; i < conv1::C_in; ++i) {
      for (int j = 0; j < conv1::W_in; ++j) {
        for (int k = 0; k < conv1::H_in; ++k) {
#pragma HLS PIPELINE II=1
          const int i_idx = i * conv1::W_in * conv1::H_in + i * conv1::W_in + k;
          fmi[i][j][k] = fm_in[i_idx];
        }
      }
    }
#ifndef __VITIS_HLS__
  };
#endif
  auto max_out = [&]() {
#pragma HLS INLINE
    ActivationType max_val = -(1 << (sizeof(ActivationType) * 8 - 1));
    // std::cout << "MAX: " << max_val << "\n";
    int max_idx = 0;
    MaxOut:
    for (int i = 0; i < dense2::L_out; ++i) {
#pragma HLS PIPELINE II=1
      if (fm5[i] > max_val) {
        max_val = fm5[i];
        max_idx = i;
      }
    }
    action = max_idx;
  };
#ifndef __VITIS_HLS__
  dma_in();
#endif
  Convolution2D<conv1>(fmi, conv1_w, conv1_b, fm1);
  Convolution2D<conv2>(fm1, conv2_w, conv2_b, fm2);
  Convolution2D<conv3>(fm2, conv3_w, conv3_b, fm3);
  FlattenDense<conv3, dense1>(fm3, dense1_w, dense1_b, fm4);
  Dense<dense2>(fm4, dense2_w, dense2_b, fm5);
  max_out();
}