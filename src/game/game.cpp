#include "game.h"
#include "ap_int.h"

void hls_game(const int action_player1, const int action_player2, unsigned char* img) {
  const int W = 64;
  const int H = 64;
#pragma HLS INTERFACE m_axi port=img offset=slave depth=W*H bundle=img_dmem
  static int num_steps = 0;
  const int screen_h = H;
  const int screen_w = W;
  const int score_w = 8;
  const int score_h = 8;
  const int bar_w = 2;
  const int bar_h = 6;
  const int ball_x = H / 2;
  const int ball_y = W / 2;
  const ap_fixed<8, 4> ball_speed = 3;
  const ap_fixed<8, 4> ball_max_bounce_angle = 80;
  const ap_fixed<8, 4> ball_time = 1;
  typedef Game<H, W, GameRealType, unsigned char> EnvType;
  static EnvType env = EnvType(screen_h, screen_w, score_w, score_h, bar_w,
    bar_h, ball_x, ball_y, ball_speed, ball_max_bounce_angle, ball_time);
  env.step(action_player1, action_player2);
  env.draw();
  ++num_steps;
  StreamOut:
  for (int i = 0; i < env._screen_h * env._screen_w; ++i) {
#pragma HLS PIPELINE II=1
    img[i] = env._state.information._buffer[i];
  }
}