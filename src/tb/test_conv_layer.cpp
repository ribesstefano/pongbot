#include "tb/test_conv_layer.h"

int main(int argc, char const *argv[]) {
  const int C_in = IMAGE_W;
  const int R_in = IMAGE_H;
  const int channels_in = IMAGE_C;
  const int kernel_size = KERNEL_SIZE;
  const int padding_size = 0;
  const int stride = STRIDE;
  const int num_kernels = NUM_KERNELS;
  const int R = int((R_in - kernel_size + 2 * padding_size) / stride + 1);
  const int C = int((C_in - kernel_size + 2 * padding_size) / stride + 1);
  ActivationType *fm_in = new ActivationType[channels_in * C_in * R_in];
  ActivationType *fm_out = new ActivationType[num_kernels * C * R];
  ActivationType *fm_out_gold = new ActivationType[num_kernels * C * R];
  WeightType *weight = new WeightType[num_kernels * channels_in * kernel_size * kernel_size];
  WeightType *bias = new WeightType[num_kernels]; // unused for now
  srand(time(NULL));
  for (int i = 0; i < channels_in * C_in * R_in; ++i) {
    fm_in[i] = ActivationType(rand() + 1);
  }
  for (int i = 0; i < num_kernels * C * R; ++i) {
    fm_out[i] = 0;
    fm_out_gold[i] = 0;
  }
  for (int i = 0; i < num_kernels * channels_in * kernel_size * kernel_size; ++i) {
    weight[i] = WeightType(rand() + 1);
  }
  for (int i = 0; i < num_kernels; ++i) {
    bias[i] = WeightType(rand() + 1);
  }

  std::cout << "[INFO] Total num of iterations: "
            << R * C * channels_in * num_kernels * kernel_size * kernel_size
            << std::endl;


  std::cout << "[INFO] Starting conv()." << std::endl;
  conv(fm_in, weight, bias, fm_out);
  std::cout << "[INFO] Starting conv_gold()." << std::endl;
  conv_gold(fm_in, weight, bias, fm_out_gold);
  int num_errors = 0;
  int err_idx = 0;
  float avg_diff = 0;
  for (int i = 0; i < num_kernels * C * R; ++i) {
    if (fm_out[i] != fm_out_gold[i]) {
      avg_diff += float((fm_out[i] - fm_out_gold[i]));
      if (num_errors == 0 || num_errors == 1) {
        // std::cout << "[ERROR] Error n." << i << ", real/gold: " << fm_out[i]
        //           << " / " << fm_out_gold[i] << std::endl;
      }
      ++num_errors;
      err_idx = i;
    }
  }
  if (num_errors > 0) {
    std::cout << "[ERROR] Last error at index: " << err_idx << std::endl;
    std::cout << "[ERROR] Avg diff: " << avg_diff / float(num_errors) << std::endl;
  }
  std::cout << "[INFO] Num errors: " << num_errors << " ("
            << float(num_errors) / float(num_kernels * C * R) * 100.0 << "%)"
            << std::endl;


  // ActivationType image[IMAGE_C][IMAGE_H][IMAGE_W] = {ActivationType(rand())};
  ActivationType image[IMAGE_C * IMAGE_H * IMAGE_W] = {ActivationType(rand())};
  int action = 0;
  conv_net(image, action);
  std::cout << action << std::endl;

  return num_errors;
}
