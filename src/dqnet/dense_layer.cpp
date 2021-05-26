#include "dqnet/dense_layer.h"
#include "ap_int.h"

typedef ConvParams<32, 3, 2, 4, 64, 64, 0, ap_fixed<16,3>, ap_fixed<16,3>, ap_fixed<16,3> > conv_test;
const int kFlattenSize = conv_test::C_out * conv_test::W_out * conv_test::H_out;
typedef DenseParams<kFlattenSize, 128, ap_fixed<16,3>, ap_fixed<16,3>, ap_fixed<16,3> > dense_test;

void hls_dense(const typename dense_test::Din fm_in[dense_test::L_in],
    const typename dense_test::Dw w[dense_test::L_out][dense_test::L_in],
    const typename dense_test::Dw bias[dense_test::L_out],
    typename dense_test::Dout fm_out[dense_test::L_out]) {
  // Interface
#pragma HLS INTERFACE bram port=fm_in
#pragma HLS INTERFACE bram port=w
#pragma HLS INTERFACE bram port=bias
#pragma HLS INTERFACE bram port=fm_out
  // Memory Partition
// #pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=3 dim=2
// #pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=3 dim=3
// #pragma HLS ARRAY_PARTITION variable=w complete dim=3
// #pragma HLS ARRAY_PARTITION variable=w complete dim=4
// #pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=2
// #pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=3
  Dense<dense_test>(fm_in, w, bias, fm_out);
}

void hls_flatten_dense(
    const typename conv_test::Din fm_in[conv_test::C_out][conv_test::W_out][conv_test::H_out],
    const typename dense_test::Dw w[dense_test::L_out][dense_test::L_in],
    const typename dense_test::Dw bias[dense_test::L_out],
    typename dense_test::Dout fm_out[dense_test::L_out]) {
// Interface
#pragma HLS INTERFACE bram port=fm_in
#pragma HLS INTERFACE bram port=w
#pragma HLS INTERFACE bram port=bias
#pragma HLS INTERFACE bram port=fm_out
  // Memory Partition
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=conv_test::K dim=2
#pragma HLS ARRAY_PARTITION variable=fm_in cyclic factor=conv_test::K dim=3
// #pragma HLS ARRAY_PARTITION variable=w complete dim=3
// #pragma HLS ARRAY_PARTITION variable=w complete dim=4
// #pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=2
// #pragma HLS ARRAY_PARTITION variable=fm_out cyclic factor=conv_test::K dim=3
  FlattenDense<conv_test, dense_test>(fm_in, w, bias, fm_out);
}
