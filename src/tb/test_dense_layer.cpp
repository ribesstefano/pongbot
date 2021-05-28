#include "tb/test_dense_layer.h"
#include "ap_int.h"

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  typedef ap_fixed<16, 3> test_t;
  typedef ConvParams<32, 3, 2, 4, 64, 64, 0, test_t, test_t, test_t > conv_p;
  const int kFlattenSize = conv_p::C_out * conv_p::W_out * conv_p::H_out;
  typedef DenseParams<kFlattenSize, 128, test_t, test_t, test_t > dense_p;
  int num_errors = 0;

  auto fm_in_conv = new test_t[conv_p::C_out][conv_p::W_out][conv_p::H_out];
  test_t* fm_in_dense = new test_t[dense_p::L_in];
  test_t* bias = new test_t[dense_p::L_out];
  test_t* fm_out_gold = new test_t[dense_p::L_out];
  test_t* fm_out_dense = new test_t[dense_p::L_out];
  test_t* fm_out_flatten = new test_t[dense_p::L_out];
  auto w = new test_t[dense_p::L_out][dense_p::L_in];
  // Init test vectors
  for (int i = 0; i < dense_p::L_out; ++i) {
    for (int j = 0; j < dense_p::L_in; ++j) {
      w[i][j] = rand();
    }
  }
  for (int wd = 0; wd < conv_p::W_out; ++wd) {
    for (int ht = 0; ht < conv_p::H_out; ++ht) {
      for (int ch = 0; ch < conv_p::C_out; ++ch) {
        const int w_idx = wd * conv_p::H_out * conv_p::C_out + ht * conv_p::C_out + ch;
        fm_in_dense[w_idx] = rand();
        fm_in_conv[ch][wd][ht] = fm_in_dense[w_idx];
      }
    }
  }
  for (int i = 0; i < dense_p::L_out; ++i) {
    bias[i] = rand();
    fm_out_dense[i] = 0;
    fm_out_flatten[i] = 0;
    fm_out_gold[i] = 0;
  }
  Dense<dense_p>(fm_in_dense, w, bias, fm_out_dense);
  FlattenDense<conv_p, dense_p>(fm_in_conv, w, bias, fm_out_flatten);
  // Golden function
  for (int i = 0; i < dense_p::L_out; ++i) {
    for (int j = 0; j < dense_p::L_in; ++j) {
      fm_out_gold[i] += fm_in_dense[j] * w[i][j];
    }
    fm_out_gold[i] += bias[i];
  }
  // Check results
  for (int i = 0; i < dense_p::L_out; ++i) {
    if (fm_out_dense[i] != fm_out_gold[i]) {
      ++num_errors;
    }
    if (fm_out_flatten[i] != fm_out_gold[i]) {
      ++num_errors;
    }
  }
  if (num_errors) {
    std::cout << "[ERROR] Number of errors: " << num_errors << std::endl;
  } else {
    std::cout << "[INFO] Test passed." << std::endl;
  }
  return num_errors;
}