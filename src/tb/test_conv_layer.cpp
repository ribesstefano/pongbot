#include "tb/test_conv_layer.h"
#include "ap_int.h"

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  typedef ap_fixed<16, 3> test_t;
  typedef ConvParams<32, 3, 2, 4, 64, 64, 0, test_t, test_t, test_t > conv_p;
  int num_errors = 0;

  auto fm_in = new test_t[conv_p::C_in][conv_p::W_in][conv_p::H_in];
  auto w = new test_t[conv_p::C_out][conv_p::C_in][conv_p::K][conv_p::K];
  auto fm_out_gold = new test_t[conv_p::C_out][conv_p::W_out][conv_p::H_out];
  auto fm_out = new test_t[conv_p::C_out][conv_p::W_out][conv_p::H_out];
  test_t* bias = new test_t[conv_p::C_out];
  // Init test vectors
  for(int to = 0; to < conv_p::C_out; to++) {
    for(int ti = 0; ti < conv_p::C_in; ti++) {
      for(int i = 0; i < conv_p::K; i++) {
        for(int j = 0; j < conv_p::K; j++) {
          w[to][ti][i][j] = rand();
        }
      }
    }
  }
  for (int wd = 0; wd < conv_p::W_out; ++wd) {
    for (int ht = 0; ht < conv_p::H_out; ++ht) {
      for (int ch = 0; ch < conv_p::C_out; ++ch) {
        fm_out[ch][wd][ht] = 0;
        fm_out_gold[ch][wd][ht] = 0;
      }
    }
  }
  for (int wd = 0; wd < conv_p::W_in; ++wd) {
    for (int ht = 0; ht < conv_p::H_in; ++ht) {
      for (int ch = 0; ch < conv_p::C_in; ++ch) {
        fm_in[ch][wd][ht] = rand();
      }
    }
  }
  for (int i = 0; i < conv_p::C_out; ++i) {
    bias[i] = rand();
  }
  Convolution2D<conv_p>(fm_in, w, bias, fm_out);
  Convolution2D_Reference<conv_p>(fm_in, w, bias, fm_out_gold);
  for (int wd = 0; wd < conv_p::W_out; ++wd) {
    for (int ht = 0; ht < conv_p::H_out; ++ht) {
      for (int ch = 0; ch < conv_p::C_out; ++ch) {
        if (fm_out[ch][wd][ht] != fm_out_gold[ch][wd][ht]) {
          ++num_errors;
        }
      }
    }
  }
  if (num_errors) {
    std::cout << "[ERROR] Number of errors: " << num_errors << std::endl;
  } else {
    std::cout << "[INFO] Test passed." << std::endl;
  }
  return num_errors;
  // const int C_in = IMAGE_W;
  // const int R_in = IMAGE_H;
  // const int channels_in = IMAGE_C;
  // const int kernel_size = CONV_KERNEL_SIZE;
  // const int padding_size = 0;
  // const int stride = CONV_STRIDE;
  // const int num_kernels = CONV_NUM_KERNELS;
  // const int R = int((R_in - kernel_size + 2 * padding_size) / stride + 1);
  // const int C = int((C_in - kernel_size + 2 * padding_size) / stride + 1);
  // ActivationType *fm_in = new ActivationType[channels_in * C_in * R_in];
  // ActivationType *fm_out = new ActivationType[num_kernels * C * R];
  // ActivationType *fm_out_gold = new ActivationType[num_kernels * C * R];
  // WeightType *weight = new WeightType[num_kernels * channels_in * kernel_size * kernel_size];
  // WeightType *bias = new WeightType[num_kernels]; // unused for now
  // srand(time(NULL));
  // for (int i = 0; i < channels_in * C_in * R_in; ++i) {
  //   fm_in[i] = ActivationType(rand() + 1);
  // }
  // for (int i = 0; i < num_kernels * C * R; ++i) {
  //   fm_out[i] = 0;
  //   fm_out_gold[i] = 0;
  // }
  // for (int i = 0; i < num_kernels * channels_in * kernel_size * kernel_size; ++i) {
  //   weight[i] = WeightType(rand() + 1);
  // }
  // for (int i = 0; i < num_kernels; ++i) {
  //   bias[i] = 0; // WeightType(rand() + 1);
  // }

  // std::cout << "[INFO] Total num of iterations: "
  //           << R * C * channels_in * num_kernels * kernel_size * kernel_size
  //           << std::endl;


  // std::cout << "[INFO] Starting conv()." << std::endl;
  // conv(fm_in, weight, bias, fm_out);
  // std::cout << "[INFO] Starting conv_gold()." << std::endl;
  // conv_gold(fm_in, weight, bias, fm_out_gold);
  // int num_errors = 0;
  // int err_idx = 0;
  // float avg_diff = 0;
  // for (int i = 0; i < num_kernels * C * R; ++i) {
  //   if (fm_out[i] != fm_out_gold[i]) {
  //     avg_diff += float((fm_out[i] - fm_out_gold[i]));
  //     if (num_errors == 0 || num_errors == 1) {
  //       // std::cout << "[ERROR] Error n." << i << ", real/gold: " << fm_out[i]
  //       //           << " / " << fm_out_gold[i] << std::endl;
  //     }
  //     ++num_errors;
  //     err_idx = i;
  //   }
  // }
  // if (num_errors > 0) {
  //   std::cout << "[ERROR] Last error at index: " << err_idx << std::endl;
  //   std::cout << "[ERROR] Avg diff: " << avg_diff / float(num_errors) << std::endl;
  // }
  // std::cout << "[INFO] Num errors: " << num_errors << " ("
  //           << float(num_errors) / float(num_kernels * C * R) * 100.0 << "%)"
  //           << std::endl;
  // return num_errors;
}
