#include "tb/test_dqnet.h"

int main(int argc, char const *argv[]) {
  // ActivationType image[IMAGE_C][IMAGE_H][IMAGE_W] = {ActivationType(rand())};
  ActivationType image[IMAGE_C * IMAGE_H * IMAGE_W] = {ActivationType(rand())};
  int action = DQNet(image);
  std::cout << action << std::endl;
  return 0;
}