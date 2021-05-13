#include "pong.h"
#include "hls_opencv.h" // This must be included in the testbench ONLY.

int main(int argc, char const *argv[]) {
  AxiStreamRGB pongStream;
  unsigned int player1_score = 0;
  unsigned int player2_score = 0;
  IplImage* pongImage = cvCreateImage(cvSize(MAX_WIDTH, MAX_HEIGHT), 8, 3);
  IplImage* hlsImage = cvCreateImage(cvSize(MAX_WIDTH, MAX_HEIGHT), 8, 3);
  cvSet(pongImage, cvScalar(0, 0, 0));
  const int steps = 8;
  for (int i = 0; i < steps; ++i) {
    std::cout << "Image n." << i << ")\n";
    const bool player_1_move_right = false; // int(rand()) % 1 == 0 ? true : false;
    const bool player_1_move_left = true; // player_1_move_right ? false : int(rand()) % 1;
    const bool player_2_move_right = false; // int(rand()) % 1 == 0 ? true : false;
    const bool player_2_move_left = true; // player_2_move_right ? false : int(rand()) % 1;
    std::cout << "Player 1: moves " << (player_1_move_left ? "L" : (player_1_move_right ? "R" : "0")) << "\n";
    std::cout << "Player 2: moves " << (player_2_move_left ? "L" : (player_2_move_right ? "R" : "0")) << "\n";
    pong(player_1_move_left, player_1_move_right, player_2_move_left,
      player_2_move_right, player1_score, player2_score, pongStream);
    AXIvideo2IplImage(pongStream, pongImage);
    std::string filename(std::string(IMAGE_OUTPUT_PATH) + "/pong_" + std::to_string(i) + ".bmp");
    cvSaveImage(filename.c_str(), pongImage);
    std::cout << "\n";
  }
  cvReleaseImage(&hlsImage);
  cvReleaseImage(&pongImage);
  return 0;
}