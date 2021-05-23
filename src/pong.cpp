#include<iostream>
#include "pong.h"

void pong_kernel(const int player_action, int& dqnet_action,
    HlsPongParams::EnvType& env, DQNet& dqnet, AxiStreamRGB &output_stream) {
#pragma HLS DATAFLOW
  const bool kResetImage = true;
  typename HlsPongParams::OutImageType output_img;
#pragma HLS STREAM variable=output_img depth=2
  // env.step(player_action, dqnet_action);
  // env.draw();
  dqnet_action = dqnet.call(env.get_observation());
  env.draw_cv_mat(env._state.information, output_img, kResetImage);
  hls::Mat2AXIvideo<24, HlsPongParams::H_out, HlsPongParams::W_out>(output_img, output_stream);
}

void hls_pong(const WeightType* dmem, const bool player1_up,
    const bool player1_down, AxiStreamRGB &output_stream) {
// #pragma HLS DATAFLOW
#pragma HLS INTERFACE s_axilite port=return bundle=ctrl
#pragma HLS INTERFACE m_axi port=dmem offset=slave depth=DQNet::size bundle=dmem
#pragma HLS INTERFACE ap_none port=player1_up  
#pragma HLS INTERFACE ap_none port=player1_down  
#pragma HLS INTERFACE axis port=output_stream
  const bool kResetImage = true;
  typename HlsPongParams::OutImageType output_img;
#pragma HLS STREAM variable=output_img depth=2
  // Initialize Game and DQNet
  static HlsPongParams::EnvType env = HlsPongParams::EnvType(
    HlsPongParams::screen_h, HlsPongParams::screen_w, HlsPongParams::score_w,
    HlsPongParams::score_h, HlsPongParams::bar_w, HlsPongParams::bar_h,
    HlsPongParams::ball_x, HlsPongParams::ball_y, HlsPongParams::ball_speed,
    HlsPongParams::ball_max_bounce_angle, HlsPongParams::ball_time);
  static DQNet dqnet = DQNet(dmem);
  // Determine player action and init DQNet action
  int player_action = 0;
  if (player1_up) {
    player_action = 1;
  } else if (player1_down) {
    player_action = -1;
  }

  static int dqnet_action = 0;
  env.step(player_action, dqnet_action);
  env.draw();
  pong_kernel(player_action, dqnet_action, env, dqnet, output_stream);

  // env.step(player_action, dqnet_action);
  // env.draw();
  // dqnet_action = dqnet.call(env.get_observation());
  // env.draw_cv_mat(env._state.information, output_img, kResetImage);
  // hls::Mat2AXIvideo<24, HlsPongParams::H_out, HlsPongParams::W_out>(output_img, output_stream);
}