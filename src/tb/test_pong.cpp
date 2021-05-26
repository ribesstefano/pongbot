#include "tb/test_pong.h"
#include "hls_opencv.h"
#include <cstdlib>

int main(int argc, char const *argv[]) {
  int num_test_steps = 16;
  WeightType* dmem = new WeightType[HlsPongParams::DQNetType::size];
  for (int i = 0; i < HlsPongParams::DQNetType::size; ++i) {
    dmem[i] = WeightType(rand());
  }
  AxiStreamRGB output_stream;
  HlsPongParams::OutImageType output_img;
  cv::Mat output_cv_img = cv::Mat(HlsPongParams::H_out, HlsPongParams::W_out, CV_8UC1);
  bool player1_up = false;
  bool player1_down = false;
  const int choices[] = {0, 1, 2};
  for (int i = 0; i < num_test_steps; ++i) {
    const int choice = choices[int(rand()) % 3];
    switch (choice) {
      case 0:
        player1_up = true;
        player1_down = false;
        break;
      case 1:
        player1_up = false;
        player1_down = false;
        break;
      case 2:
        player1_up = false;
        player1_down = true;
        break;
    }
    hls_pong(dmem, player1_up, player1_down, output_stream);
    // AXI-Stream -> hls::Mat -> cv::Mat -> image file
    hls::AXIvideo2Mat(output_stream, output_img);
    hlsMat2cvMat(output_img, output_cv_img);
    std::string filename(std::string(IMAGE_OUTPUT_PATH) + "/pong_test_" + std::to_string(i) + ".bmp");
    cv::imwrite(filename.c_str(), output_cv_img);
  }
  delete[] dmem;
  return 0;
}