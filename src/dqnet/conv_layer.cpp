#include "dqnet/conv_layer.h"
#include "hls_math.h"
#include "hls_half.h"
#include "ap_int.h"

typedef ap_fixed<16, 3> ActivationType;
typedef ap_fixed<8, 3> WeightType;

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
        const int out_idx = to * R * C + row * C + col;
        for(int ti = 0; ti < N; ti++) { // N: input channels
#pragma HLS PIPELINE II=1
          for(int i = 0; i < K; i++) {
            for(int j = 0; j < K; j++) {
              const int in_idx = ti * R_in * C_in + (S * row + i) * C_in + S * col + j;
              const int w_idx = to * N * K * K + ti * K * K + i * K + j;
              if (S * row + i > R_in || S * col + j > C_in) {
                std::cout << "\t\tout o bounds" << "\n";
              }
              fm_out[out_idx] += weight[w_idx] * fm_in[in_idx];
            }
          }
        }
        // fm_out[out_idx] += bias[out_idx]; // TODO: Seg fault. Fix this.
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
  const int R = int((R_in - K + 2 * P) / S + 1);
  const int C = int((C_in - K + 2 * P) / S + 1);
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

typedef ConvParams<32, 3, 2, 4, 64, 64, 0, ap_fixed<16,3>, ap_fixed<16,3>, ap_fixed<16,3> > conv_test;

void hls_conv(
    const typename conv_test::Din fm_in[conv_test::C_in][conv_test::W_in][conv_test::H_in],
    const typename conv_test::Dw w[conv_test::C_out][conv_test::C_in][conv_test::K][conv_test::K],
    const typename conv_test::Dw bias[conv_test::C_out],
    typename conv_test::Dout fm_out[conv_test::C_out][conv_test::W_out][conv_test::H_out]) {
  // Interface
#pragma HLS INTERFACE bram port=fm_in
#pragma HLS INTERFACE bram port=w
#pragma HLS INTERFACE bram port=bias
#pragma HLS INTERFACE bram port=fm_out
  // Memory Partition
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=conv_test::K dim=2
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=conv_test::K dim=3
#pragma HLS ARRAY_PARTITION variable=w complete dim=3
#pragma HLS ARRAY_PARTITION variable=w complete dim=4
#pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=2
#pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=3
  Convolution2D<conv_test>(fm_in, w, bias, fm_out);
}