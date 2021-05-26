#ifndef GAME_PONG_H_
#define GAME_PONG_H_

#include "game/game.h"
#include "dqnet/dqnet.h"

typedef struct _HlsPongParams {
  static const int C = 1;
  static const int W = 64;
  static const int H = 64;
  static const int W_out = 800;
  static const int H_out = 600;
  static const int screen_h = H;
  static const int screen_w = W;
  static const int score_w = W / 8;
  static const int score_h = H / 8;
  static const int bar_w = score_w / 4;
  static const int bar_h = score_h / 4 * 3;
  static const int ball_x = H / 2;
  static const int ball_y = W / 2;
  static const unsigned char ball_speed = 3;
  static const unsigned char ball_max_bounce_angle = 80;
  static const unsigned char ball_time = 1;
  typedef hls::Mat<H_out, W_out, HLS_8UC1> OutImageType;
  typedef Game<H, W, GameRealType, unsigned char> EnvType;
  typedef DQNet<C, H, W> DQNetType;
} HlsPongParams;

void hls_pong(const WeightType* dmem, const bool player1_up,
    const bool player1_down, AxiStreamRGB &output_stream);

#endif // end GAME_PONG_H_