#include "hdmi_out.h"
#include "axis_lib.h"

// void painter(AxiStreamRGB &tpgStream, AxiStreamRGB &maskStream, AxiStreamRGB &paintedStream) {
// #pragma HLS INTERFACE axis port=paintedStream bundle=OUT_STREAM
// #pragma HLS INTERFACE axis port=maskStream bundle=MASK_STREAM
// #pragma HLS INTERFACE axis port=tpgStream bundle=TPG_STREAM
// #pragma HLS INTERFACE s_axilite port=return bundle=CONTORL_BUS
// #pragma HLS DATAFLOW
//   ImageRGB video_in(MAX_HEIGHT, MAX_WIDTH);
//   ImageRGB mask_in(MAX_HEIGHT, MAX_WIDTH);
//   ImageRGB painted_out(MAX_HEIGHT, MAX_WIDTH);
//   hls::AXIvideo2Mat(tpgStream, video_in);
//   hls::AXIvideo2Mat(maskStream, mask_in);
//   hls::Mul(mask_in, video_in, painted_out);
//   hls::Mat2AXIvideo(painted_out, paintedStream);
// }

// template <typename T>
// T min(T a, T b) {
// #pragma HLS INLINE
//   return a < b ? a : b;
// }

// template <typename T>
// T max(T a, T b) {
// #pragma HLS INLINE
//   return a > b ? a : b;
// }

// void update_player_position(const bool move_up, const bool move_down,
//     const int bar_heigth, const int mov_size, const int screen_heigth,
//     unsigned int& player_y) {
// #pragma HLS INLINE
// #pragma HLS PIPELINE II=1
//   int movement;
//   int player_y_tmp = player_y;
//   if (move_up) {
//     movement = -mov_size;
//   } else if (move_down) {
//     movement = mov_size;
//   } else {
//     movement = 0; // don't make any movement
//   }
//   player_y_tmp = min(player_y_tmp + movement, screen_heigth - bar_heigth);
//   player_y_tmp = max(player_y_tmp, 0);
//   player_y = player_y_tmp;
// }

// void pong(const bool player1_move_left, const bool player1_move_right,
//   const bool player2_move_left, const bool player2_move_right,
//   unsigned int &player1_score, unsigned int &player2_score,
//   AxiStreamRGB &outStream) {
// // #pragma HLS INTERFACE ap_ctrl_chain port=return
// #pragma HLS INTERFACE s_axilite port=return bundle=CONTORL_BUS
// #pragma HLS INTERFACE s_axilite port=player1_score bundle=CONTORL_BUS
// #pragma HLS INTERFACE s_axilite port=player2_score bundle=CONTORL_BUS
// #pragma HLS INTERFACE axis port=outStream bundle=OUT_STREAM
// #pragma HLS DATAFLOW

//   const int kWidth = MAX_WIDTH;
//   const int kHeight = MAX_HEIGHT;
//   const int kBarWidth = kWidth * 0.3;
//   const int kBarHeight = kHeight * 0.05;
//   const int kBallRadius = 20;

//   typedef ap_axiu<24, 1, 1, 1> AxiPacket;
//   const unsigned int kBlackPixel = 0;
//   const unsigned int kWhitePixel = 0xFFFFFF;

//   // Start in the middle
//   static unsigned int player1_y = kWidth / 2 - kBarWidth / 2;
//   static unsigned int player2_y = kWidth / 2 - kBarWidth / 2;

//   static unsigned int _player1_score = 0;
//   static unsigned int _player2_score = 0;

//    // For the points inside the ball object to be synthesizeable.
//   static int tail_buffer[2];
//   static int arrow_buffer[2];
// #pragma HLS ARRAY_PARTITION variable=tail_buffer complete dim=0
// #pragma HLS ARRAY_PARTITION variable=arrow_buffer complete dim=0
//   static Ball<kBallRadius> ball(kHeight, kWidth, kBarHeight, kBarWidth, tail_buffer, arrow_buffer);

//   // Update player position
//   update_player_position(player1_move_left, player1_move_right, kBarWidth,
//     kBallRadius, kWidth, player1_y);
//   update_player_position(player2_move_left, player2_move_right, kBarWidth,
//     kBallRadius, kWidth, player2_y);

//   int losing_player_id = ball.update_ball_position(player1_y - kWidth / 2, player2_y - kWidth / 2);

//   for (int y = 0; y < kHeight; ++y) {
//     for (int x = 0; x < kWidth; ++x) {
// #pragma HLS loop_flatten off // As specified in "hls_video_io.h"
// #pragma HLS PIPELINE II=1
//       AxiPacket output_packet;
//       output_packet.data = kBlackPixel;
//       if (losing_player_id) {
//         player1_y = kWidth / 2 - kBarWidth / 2;
//         player2_y = kWidth / 2 - kBarWidth / 2;
//         _player1_score += losing_player_id == 1 ? 1 : 0;
//         _player2_score += losing_player_id == 2 ? 1 : 0;
//         ball.reset_ball_position();
//       } else {
//         // Keep playing
//         if (y < kBarHeight || y > kHeight - 1 - kBarHeight) {
//           const unsigned int player_y = y < kBarHeight ? player1_y : player2_y;
//           if (x > player_y && x < player_y + kBarWidth) {
//             output_packet.data = kWhitePixel;
//           } else {
//             output_packet.data = kBlackPixel;
//           }
//         } else {
//           // Ball drawing
//           output_packet.data = ball.is_ball_pixel(x, y) ? kWhitePixel : kBlackPixel;
//         }
//       }
//       // Configure TLAST and other AXIS protocol signals.
//       if (x == kWidth - 1) {
//         output_packet.last = 1;
//       } else {
//         output_packet.last = 0;
//       }
//       if (y == 0 && x == 0) {
//         output_packet.user = 1;
//       } else {
//         output_packet.user = 0;
//       }
//       outStream << output_packet;
//     }
//   }
//   player1_score = _player1_score;
//   player2_score = _player2_score;
// #ifndef __SYNTHESIS__
//   std::cout << "Transfering done.\n";
// #endif
// }