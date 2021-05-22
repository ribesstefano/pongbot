#include<iostream>
#include "pong.h"

void hls_pong(const WeightType* dmem, const bool player1_up,
    const bool player1_down, AxiStreamRGB &output_stream) {
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE m_axi port=dmem offset=slave depth=1 bundle=dmem
#pragma HLS INTERFACE ap_none port=player1_up  
#pragma HLS INTERFACE ap_none port=player1_down  
#pragma HLS INTERFACE axis port=output_stream
  const int W = 64;
  const int H = 64;
  const int W_out = 800;
  const int H_out = 600;
  typedef hls::Mat<H_out, W_out, HLS_8UC1> OutImageType;
  typedef Game<H, W, GameRealType, unsigned char> EnvType;
  OutImageType output_img;
  const bool kResetImage = true;
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
  // Initialize Game and DQNet
  static EnvType env = EnvType(screen_h, screen_w, score_w, score_h, bar_w,
    bar_h, ball_x, ball_y, ball_speed, ball_max_bounce_angle, ball_time);
  static DQNet dqnet = DQNet(dmem);
  // Determine player action and init DQNet action
  int player_action = 0;
  static int dqnet_action;
  static bool init_dqnet_action = true;
  if (player1_up) {
    player_action = 1;
  } else if (player1_down) {
    player_action = -1;
  }
  if (init_dqnet_action) {
    dqnet_action = 0;
    init_dqnet_action = false;
  }
  env.step(player_action, dqnet_action);
  env.draw();
  if (!init_dqnet_action) {
    dqnet_action = dqnet.call(env.get_observation());
  }
  env.draw_cv_mat<OutImageType>(env._state.information, output_img, kResetImage);
  hls::Mat2AXIvideo<24, H_out, W_out>(output_img, output_stream);
}