#include "tb/test_game.h"
#include "hls_opencv.h"

int main(int argc, char const *argv[]) {
  const int W = 64;
  const int H = 64;
  const int num_steps = 8;

  const unsigned char screen_h = H;
  const unsigned char screen_w = W;
  const unsigned char score_w = 8;
  const unsigned char score_h = 8;
  const unsigned char bar_w = 2;
  const unsigned char bar_h = 6;
  const unsigned char ball_x = H / 2;
  const unsigned char ball_y = W / 2;
  const unsigned char ball_speed = 3;
  const unsigned char ball_max_bounce_angle = 80;
  const unsigned char ball_time = 1;
  Game<H, W, unsigned char> env = Game<H, W, unsigned char>(screen_h, screen_w,
    score_w, score_h, bar_w, bar_h, ball_x, ball_y, ball_speed,
    ball_max_bounce_angle, ball_time);

  for (int i = 0; i < num_steps; ++i) {
    auto tmp = env.step(1, -1);
    env.draw(true);
    std::string filename(std::string(IMAGE_OUTPUT_PATH) + "/game_" + std::to_string(i) + ".bmp");
    cv::Mat gameImage = cv::Mat(H, W, CV_8UC1, env.screen_img_.buffer_);
    cv::imwrite(filename.c_str(), gameImage);
  }

  // IplImage* gameImage = cvCreateImage(cvSize(W, H), 8, 1);
  // cvSet(gameImage, cvScalar(0, 0, 0));
  // memcpy(gameImage->imageData, game.screen_img_.buffer_, sizeof(unsigned char) * H * W);
  // cvSaveImage(filename.c_str(), gameImage);
  // cvReleaseImage(&gameImage);
  
  // for (int i = 0; i < num_steps; ++i) {
  //   std::cout << "Image n." << i << ")\n";
  //   const bool player_1_move_right = false; // int(rand()) % 1 == 0 ? true : false;
  //   const bool player_1_move_left = true; // player_1_move_right ? false : int(rand()) % 1;
  //   const bool player_2_move_right = false; // int(rand()) % 1 == 0 ? true : false;
  //   const bool player_2_move_left = true; // player_2_move_right ? false : int(rand()) % 1;
  //   std::cout << "Player 1: moves " << (player_1_move_left ? "L" : (player_1_move_right ? "R" : "0")) << "\n";
  //   std::cout << "Player 2: moves " << (player_2_move_left ? "L" : (player_2_move_right ? "R" : "0")) << "\n";
  //   pong(player_1_move_left, player_1_move_right, player_2_move_left,
  //     player_2_move_right, player1_score, player2_score, pongStream);
  //   AXIvideo2IplImage(pongStream, pongImage);
  //   std::cout << "\n";
  // }
  
  return 0;
}