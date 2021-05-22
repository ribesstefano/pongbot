#ifndef GAME_GAME_H_
#define GAME_GAME_H_

#include <hls_stream.h>
#include <hls_video.h> // This must be included in the source/syn files only
#include <cassert>

#include "game_params.h"
#include "game_utils.h"
#include "ball.h"

template <int H, int W, typename T, typename P>
class Game {

private:
  bool slice (int i, int x0, int x1) {
    return (i >= x0 && i < x1) ? true : false;
  }

public:

  typedef ImageBuffer<H, W, P> ImageType;
  typedef hls::Scalar<1, P> HlsPixelType;
  typedef hls::Mat<H, W, HLS_8UC1> HlsImageType;

  typedef struct StateType {
    int reward_player1;
    int reward_player2;
    bool done;
    ImageType observation;
    ImageType information;
  } StateType;

  Game(const int screen_h, const int screen_w, const int score_w,
       const int score_h, const int bar_w, const int bar_h, const int ball_x,
       const int ball_y, T ball_speed, T ball_max_bounce_angle, T ball_time)
      : _ball(ball_x, ball_y, ball_time, ball_speed, ball_max_bounce_angle,
              std::max(screen_w / 64, 1)) {
    // Screen parameters
    this->_screen_h = screen_h;
    this->_screen_w = screen_w;
    this->_score_h = score_h;
    this->_score_w = score_w;
    // Ball parameters
    this->_ball_speed = ball_speed;
    this->_ball_radius = std::max(screen_w / 64, 1);
    this->_ball_max_bounce_angle = ball_max_bounce_angle;
    this->_ball_time = ball_time;
    // Bars parameters
    this->_bar_w = bar_w;
    this->_bar_h = bar_h;
    this->_wall_player1 = (this->_screen_w / 32 + this->_bar_w) + this->_bar_w;
    this->_wall_player2 = this->_screen_w - this->_wall_player1;
    // Player parameters
    this->_player1_y = this->_screen_h / 2 - this->_bar_h / 2;
    this->_player2_y = this->_screen_h / 2 - this->_bar_h / 2;
    this->_player1_score = 0;
    this->_player2_score = 0;
    this->_play_as_best_player = false;
  }
  ~Game() {}

  void set_play_best_move(const bool move) {
    this->_play_as_best_player = move;
  }

  void draw_number(const int score, const int score_y, const int score_x, ImageType& img) {
#ifndef ON
#define ON 1
#endif
#ifndef OFF
#define OFF 0
#endif
    const ap_uint<1> seven_segment_mask[10][7] = {
      {ON, ON, ON, ON, ON, ON, OFF},
      {OFF, ON, ON, OFF, OFF, OFF, OFF},
      {ON, ON, OFF, ON, ON, OFF, ON},
      {ON, ON, ON, ON, OFF, OFF, ON},
      {OFF, ON, ON, OFF, OFF, ON, ON},
      {ON, OFF, ON, ON, OFF, ON, ON},
      {ON, OFF, ON, ON, ON, ON, ON},
      {ON, ON, ON, OFF, OFF, OFF, OFF},
      {ON, ON, ON, ON, ON, ON, ON},
      {ON, ON, ON, ON, OFF, ON, ON}
    };
#pragma HLS ARRAY_PARTITION variable=seven_segment_mask complete dim=0
    const int seg_w = std::max(this->_score_h / 8, 1); // segment width

    int x_low[7], x_high[7], y_low[7], y_high[7];
    x_low[0] = seg_w;
    x_high[0] = this->_score_w - seg_w;
    y_low[0] = 0;
    y_high[0] = seg_w;
    x_low[1] = this->_score_h - seg_w;
    x_high[1] = this->_score_h;
    y_low[1] = seg_w;
    y_high[1] = this->_score_h / 2 - seg_w / 2;
    x_low[2] = this->_score_h - seg_w;
    x_high[2] = this->_score_h;
    y_low[2] = this->_score_h / 2 + seg_w / 2;
    y_high[2] = this->_score_h - seg_w;
    x_low[3] = seg_w;
    x_high[3] = this->_score_w - seg_w;
    y_low[3] = this->_score_w - seg_w;
    y_high[3] = this->_score_w;
    x_low[4] = 0;
    x_high[4] = seg_w;
    y_low[4] = this->_score_h / 2 + seg_w / 2;
    y_high[4] = this->_score_h - seg_w;
    x_low[5] = 0;
    x_high[5] = seg_w;
    y_low[5] = seg_w;
    y_high[5] = this->_score_h / 2 - seg_w / 2;
    x_low[6] = seg_w;
    x_high[6] = this->_score_w - seg_w;
    y_low[6] = this->_score_h / 2 - seg_w / 2;
    y_high[6] = this->_score_h / 2 + std::max(seg_w / 2, 1);

    for (int i = 0; i < 7; ++i) {
      for (int j = y_low[i]; j <= y_high[i]; ++j) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=y_low[i]-y_high[i]
        for (int k = x_low[i]; k <= x_high[i]; ++k) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=x_low[i]-x_high[i]
#pragma HLS PIPELINE II=1
          img[j + score_y][k + score_x] = seven_segment_mask[score][i] ? WHITE_PIXEL : BLACK_PIXEL;
        }
      }
    }
  }

  void draw_scores(ImageType& img) {
    assert(this->_player1_score <= 10);
    assert(this->_player2_score <= 10);
    const int score_left = this->_player1_score % 10;
    const int score_right = this->_player2_score % 10;
    const int score_y = this->_score_h;
    const int score_x_left = this->_screen_w / 2 - this->_score_w * 2;
    const int score_x_right = this->_screen_w / 2 + this->_score_w;
    this->draw_number(score_left, score_y, score_x_left, img);
    this->draw_number(score_right, score_y, score_x_right, img);
  }

  void draw_midfield(ImageType& img) {
    for (int i = 0; i < this->_screen_h; ++i) {
#pragma HLS PIPELINE II=1
      img[i][this->_screen_w / 2] = WHITE_PIXEL;
    }
  }

  void draw_bar(const int player_y, const int dist_from_wall, ImageType& img) {
    for (int i = player_y + 1; i <= std::min(this->_screen_h, player_y + this->_bar_h); ++i) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=this->_bar_h
      for (int j = dist_from_wall + 1; j <= dist_from_wall + this->_bar_w; ++j) {
#pragma HLS LOOP_TRIPCOUNT min=this->_bar_w max=this->_bar_w
#pragma HLS PIPELINE II=1
        img[i][j] = WHITE_PIXEL;
      }
    }
  }

  int get_follow_ball_action(const int player_id) {
    const int player_y = player_id == 0 ? this->_player1_y : this->_player2_y;
    if (this->_ball._y != player_y) {
      return int(this->_ball._y - player_y - this->_bar_h / 2);
    } else {
      return int(player_y - this->_bar_h / 2);
    }
  }
    
  void reset() {
    this->_player1_score = 0;
    this->_player2_score = 0;
    this->_player1_y = this->_screen_h / 2 - this->_bar_h / 2;
    this->_player2_y = this->_screen_h / 2 - this->_bar_h / 2;
    this->_ball.reset_pos();
    this->_state.done = false;
    // return this->draw_()
  }


  void step(const int player1_action, int player2_action) {
#pragma HLS PIPELINE II=1
    // Ignore given action for player 2 if 'best player' option is selected
    if (this->_play_as_best_player) {
      const int player_id = 1;
      player2_action = this->get_follow_ball_action(player_id);
    }
    // Ceil player 1 position
    this->_player1_y += player1_action;
    this->_player1_y = std::min(this->_player1_y, this->_screen_h - this->_bar_h);
    this->_player1_y = std::max(this->_player1_y, 0);
    // Ceil player 2 position
    this->_player2_y += player2_action;
    this->_player2_y = std::min(this->_player2_y, this->_screen_h - this->_bar_h);
    this->_player2_y = std::max(this->_player2_y, 0);
    PlayerStatusType status = this->_ball.update_pos(this->_player1_y,
      this->_player2_y, this->_wall_player1, this->_wall_player2, this->_bar_h,
      this->_screen_h);
    int reward_player1 = 0;
    int reward_player2 = 0;
    if (status.player1 == kHitBall) {
      reward_player1 = 1;
    }
    if (status.player1 == kWon) {
      reward_player1 = 1;
      this->_player1_score += 1;
    }
    else if (status.player1 == kLost) {
      reward_player1 = -1;
    }
    if (status.player2 == kHitBall) {
      reward_player2 = 1;
    }
    if (status.player2 == kWon) {
      reward_player2 = 1;
      this->_player2_score += 1;
    }
    else if (status.player2 == kLost) {
      reward_player2 = -1;
    }
    // Randomize ball x-direction when restarting
    if (status.player1 == kWon || status.player2 == kWon) {
      if (xorrand() % 2 == 0) {
        this->_ball._vx *= -1;
      } else {
        this->_ball._vx *= 1;
      }
      if (xorrand() % 2 == 0) {
        this->_ball._vy = -1;
      } else {
        this->_ball._vy = 1;
      }
    }
    // this->draw();
    this->_state.reward_player1;
    this->_state.reward_player2;
    if (this->_player1_score >= 10 || this->_player2_score >= 10) {
      this->_state.done = true;
      this->reset();
    } else {
      this->_state.done = false;
    }
  }

  void step(const int player1_action) {
#pragma HLS PIPELINE II=1
    const int player_id = 1;
    int player2_action  = this->get_follow_ball_action(player_id);
    auto tmp = this->_play_as_best_player; // Backup value
    this->_play_as_best_player = false; // To avoid recomputing best action.
    this->step(player1_action, player2_action);
    this->_play_as_best_player = tmp; // Restore
  }

  void draw_img(ImageType& img, const bool draw_scores_and_midfield = true) {
    this->draw_bar(this->_player1_y, this->_wall_player1 - this->_bar_w, img);
    this->draw_bar(this->_player2_y, this->_wall_player2, img);
    this->_ball.draw(this->_screen_h, this->_screen_w, img);
    if (draw_scores_and_midfield) {
      this->draw_midfield(img);
      this->draw_scores(img);
    }
  }

  void draw() {
#pragma HLS DATAFLOW
    this->draw_img(this->_state.information, true);
    this->draw_img(this->_state.observation, false);
  }

  void reset_draw() {
    for (int i = 0; i < this->_state.information.size; ++i) {
#pragma HLS PIPELINE II=1
      this->_state.information._buffer[i] = BLACK_PIXEL;
    }
  }

  void draw_information(HlsImageType& info_img) {
    this->draw_cv_mat(this->_state.information, info_img);
  }

  void draw_cv_mat(ImageType& img, HlsImageType& out_img, bool reset_img = false) {
    for (int i = 0; i < img.size; ++i) {
#pragma HLS PIPELINE II=1
      HlsPixelType pixel = HlsPixelType(img._buffer[i]);
      out_img << pixel;
      if (reset_img) {
        img._buffer[i] = BLACK_PIXEL;
      }
    }
  }

  template <typename ImageOut>
  void draw_cv_mat(ImageType& img, ImageOut& out_img, bool reset_img = false) {
#pragma HLS DATAFLOW
    HlsImageType tmp_img;
    for (int i = 0; i < img.size; ++i) {
#pragma HLS PIPELINE II=1
      HlsPixelType pixel = HlsPixelType(img._buffer[i]);
      tmp_img << pixel;
      if (reset_img) {
        img._buffer[i] = BLACK_PIXEL;
      }
    }
    hls::Resize(tmp_img, out_img);
  }

  void draw(HlsImageType& obs_img, HlsImageType& info_img) {
    this->draw_cv_mat(this->_state.observation, obs_img);
    this->draw_cv_mat(this->_state.information, info_img);
  }

  const P* get_information() {
    return this->_state.information._buffer;
  }

  const P* get_observation() {
    return this->_state.observation._buffer;
  }

  StateType _state;
  // Screen parameters
  int _screen_h;
  int _screen_w;
  int _score_h;
  int _score_w;
  // Ball parameters
  T _ball_speed;
  T _ball_radius;
  T _ball_max_bounce_angle;
  T _ball_time;
  Ball<T> _ball;
  // Bars parameters
  int _bar_w;
  int _bar_h;
  int _wall_player1;
  int _wall_player2;
  // Player parameters
  int _player1_y;
  int _player2_y;
  int _player1_score;
  int _player2_score;
  bool _play_as_best_player;
};

#endif // end GAME_GAME_H_