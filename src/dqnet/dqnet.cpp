#include "dqnet/dqnet.h"

WeightType conv1_w[32][4][3][3];
WeightType conv2_w[32][32][3][3];
WeightType conv3_w[32][32][3][3];
WeightType dense1_w[128][1568];
WeightType dense2_w[3][128];

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

int DQNetCall(const ActivationType *fm_in) {
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

  WeightType conv1_w[conv1::C_out][conv1::C_in][conv1::K][conv1::K];
  WeightType conv2_w[conv2::C_out][conv2::C_in][conv2::K][conv2::K];
  WeightType conv3_w[conv3::C_out][conv3::C_in][conv3::K][conv3::K];
  WeightType dense1_w[dense1::L_out][dense1::L_in];
  WeightType dense2_w[dense2::L_out][dense2::L_in];
  // std::cout << "conv1_w[" << conv1::C_out << "][" << conv1::C_in << "][" << conv1::K << "][" << conv1::K << "]" << std::endl;
  // std::cout << "conv2_w[" << conv2::C_out << "][" << conv2::C_in << "][" << conv2::K << "][" << conv2::K << "]" << std::endl;
  // std::cout << "conv3_w[" << conv3::C_out << "][" << conv3::C_in << "][" << conv3::K << "][" << conv3::K << "]" << std::endl;
  // std::cout << "dense1_w[" << dense1::L_out << "][" << dense1::L_in << "]" << std::endl;
  // std::cout << "dense2_w[" << dense2::L_out << "][" << dense2::L_in << "]" << std::endl;

  WeightType conv1_b[conv1::C_out];
  WeightType conv2_b[conv2::C_out];
  WeightType conv3_b[conv3::C_out];
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
  auto max_out_action = [&]() {
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
    return max_idx;
  };
#ifndef __VITIS_HLS__
  dma_in();
#endif
  Convolution2D<conv1>(fmi, conv1_w, conv1_b, fm1);
  Convolution2D<conv2>(fm1, conv2_w, conv2_b, fm2);
  Convolution2D<conv3>(fm2, conv3_w, conv3_b, fm3);
  FlattenDense<conv3, dense1>(fm3, dense1_w, dense1_b, fm4);
  Dense<dense2>(fm4, dense2_w, dense2_b, fm5);
  return max_out_action();
}


void hls_dqnet(const WeightType* dmem, const ActivationType *fm_in, int& dqnet_action) {
  typedef DQNet<4, 64, 64> DQNetType;
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE s_axilite port=dqnet_action bundle=ctrl
#pragma HLS INTERFACE m_axi port=dmem offset=slave depth=DQNetType::size bundle=dmem
#pragma HLS INTERFACE bram port=fm_in
#pragma HLS DATAFLOW
  static DQNetType dqnet = DQNetType(dmem);
  dqnet_action = dqnet.call<ActivationType>(fm_in);
}