#include "tb/test_game.h"
#include "hls_opencv.h"

int main(int argc, char const *argv[]) {
  const int W = 64;
  const int H = 64;
  const int num_steps = 16;

  const int screen_h = H;
  const int screen_w = W;
  const int score_w = W / 8;
  const int score_h = H / 8;
  const int bar_w = score_w / 4;
  const int bar_h = score_h / 4 * 3;
  const int ball_x = H / 2;
  const int ball_y = W / 2;
  const unsigned char ball_speed = 3;
  const unsigned char ball_max_bounce_angle = 80;
  const unsigned char ball_time = 1;
  typedef Game<H, W, GameRealType, unsigned char> EnvType;
  EnvType env = EnvType(screen_h, screen_w, score_w, score_h, bar_w, bar_h,
    ball_x, ball_y, ball_speed, ball_max_bounce_angle, ball_time);

  for (int i = 0; i < num_steps; ++i) {
    std::cout << "[INFO] Step n." << i << std::endl;
    std::cout << "[INFO] x = " << env._ball._x;
    std::cout << "\ty = " << env._ball._y << std::endl;
    env.step(1, -1);
    env.draw();
    std::string filename(std::string(IMAGE_OUTPUT_PATH) + "/game_" + std::to_string(i) + ".bmp");
    cv::Mat gameImage = cv::Mat(H, W, CV_8UC1, env._state.information._buffer);
    cv::imwrite(filename.c_str(), gameImage);
    env.reset_draw();
  }

  // IplImage* gameImage = cvCreateImage(cvSize(W, H), 8, 1);
  // cvSet(gameImage, cvScalar(0, 0, 0));
  // memcpy(gameImage->imageData, game._screen_img._buffer, sizeof(unsigned char) * H * W);
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