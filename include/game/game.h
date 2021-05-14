#ifndef GAME_GAME_H_
#define GAME_GAME_H_

#include <hls_stream.h>
#include <hls_video.h> // This must be included in the source/syn files only
#include <cassert>

#include "game_params.h"
#include "game_utils.h"
#include "ball.h"
// #include "point.h"

template <int H, int W, typename T>
class Game {

private:
  bool slice (int i, int x0, int x1) {
    return (i >= x0 && i < x1) ? true : false;
  }

public:

  typedef struct StateType {
    ImageBuffer<H, W, T> observation;
    int reward_player1;
    int reward_player2;
    bool done;
    ImageBuffer<H, W, T> info;
  } StateType;

  Game(const int screen_h, const int screen_w, const int score_w,
       const int score_h, const int bar_w, const int bar_h, const int ball_x,
       const int ball_y, T ball_speed, T ball_max_bounce_angle, T ball_time)
      : ball_(ball_x, ball_y, ball_time, ball_speed, ball_max_bounce_angle,
              std::max(screen_w / 64, 1)) {
    // Screen parameters
    this->screen_h_ = screen_h;
    this->screen_w_ = screen_w;
    this->score_h_ = score_h;
    this->score_w_ = score_w;
    // Ball parameters
    this->ball_x_ = ball_x;
    this->ball_y_ = ball_y;
    this->ball_speed_ = ball_speed;
    this->ball_radius_ = std::max(screen_w / 64, 1);
    this->ball_max_bounce_angle_ = ball_max_bounce_angle;
    this->ball_time_ = ball_time;
    // Bars parameters
    this->bar_w_ = bar_w;
    this->bar_h_ = bar_h;
    this->wall_player1_ = (this->screen_w_ / 32 + this->bar_w_) + this->bar_w_;
    this->wall_player2_ = this->screen_w_ - this->wall_player1_;
    // Player parameters
    this->player1_y_ = this->screen_h_ / 2 - this->bar_h_ / 2;
    this->player2_y_ = this->screen_h_ / 2 - this->bar_h_ / 2;
    this->player1_score_ = 0;
    this->player2_score_ = 0;
    this->play_as_best_player_ = false;
  }
  ~Game() {}

  void draw_number(const int score, const int score_y, const int score_x) {
#ifndef ON
#define ON WHITE_PIXEL
#endif
#ifndef OFF
#define OFF BLACK_PIXEL
#endif
    const unsigned char seven_segment_mask[10][7] = {
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
    const int seg_w = std::max(this->score_h_ / 8, 1); // segment width
    for (int i = 0; i < this->score_h_; ++i) {
      for (int j = 0; j < this->score_w_; ++j) {
        if (slice(i, 0, seg_w) && slice(j, seg_w, this->score_w_-seg_w)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][0];
        }
        if (slice(i, seg_w, this->score_h_/2-seg_w/2) && slice(j, this->score_h_-seg_w, this->score_h_)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][1];
        }
        if (slice(i, this->score_h_/2+seg_w/2, this->score_h_-seg_w) && slice(j, this->score_h_-seg_w, this->score_h_)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][2];
        }
        if (slice(i, this->score_w_-seg_w, this->score_w_) && slice(j, seg_w, this->score_w_-seg_w)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][3];
        }
        if (slice(i, this->score_h_/2+seg_w/2, this->score_h_-seg_w) && slice(j, 0, seg_w)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][4];
        }
        if (slice(i, seg_w, this->score_h_ / 2-seg_w/2) && slice(j, 0, seg_w)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][5];
        }
        if (slice(i, this->score_h_/2-seg_w/2, this->score_h_/2+std::max(seg_w/2,1)) && slice(j, seg_w, this->score_w_-seg_w)) {
          this->screen_img_[i + score_y][j + score_x] = seven_segment_mask[score][6];
        }
      }
    }
  }

  void draw_scores() {
    assert(this->player1_score_ <= 10);
    assert(this->player2_score_ <= 10);
    const int score_left = this->player1_score_ % 10;
    const int score_right = this->player2_score_ % 10;
    const int score_y = this->score_h_;
    const int score_x_left = this->screen_w_/2 - this->score_w_ * 2;
    const int score_x_right = this->screen_w_/2 + this->score_w_;
    this->draw_number(score_left, score_y, score_x_left);
    this->draw_number(score_right, score_y, score_x_right);
  }

  void draw_midfield() {
    for (int i = 0; i < this->screen_h_; ++i) {
      this->screen_img_[i][this->screen_w_ / 2] = WHITE_PIXEL;
    }
  }

  void draw_bar(const int player_y, const int dist_from_wall) {
    for (int i = 0; i < this->screen_h_; ++i) {
      for (int j = 0; j < this->screen_w_; ++j) {
        if (slice(i, player_y, player_y + this->bar_h_)) {
          if (slice(j, dist_from_wall, dist_from_wall + this->bar_w_)) {
            this->screen_img_[i][j] = WHITE_PIXEL;
          }
        }
      }
    }
  }

  void draw(const bool draw_score) {
#pragma HLS PIPELINE II=1
    this->draw_bar(this->player1_y_, this->wall_player1_ - this->bar_w_);
    this->draw_bar(this->player2_y_, this->wall_player2_);
    this->ball_.draw(this->screen_img_, this->screen_h_, this->screen_w_);
    if (draw_score) {
      this->draw_midfield();
      this->draw_scores();
    }
  }

  int get_follow_ball_action(const int player_id) {
    const int player_y = player_id == 0 ? this->player1_y_ : this->player2_y_;
    if (this->ball_y_ != player_y) {
      return int(this->ball_y_ - player_y - this->bar_h_ / 2);
    } else {
      return int(player_y - this->bar_h_ / 2);
    }
  }
    
  void reset() {
    this->player1_score_ = 0;
    this->player2_score_ = 0;
    this->player1_y_ = this->screen_h_ / 2 - this->bar_h_ / 2;
    this->player2_y_ = this->screen_h_ / 2 - this->bar_h_ / 2;
    this->ball_ = BallV2<T>(this->ball_x_, this->ball_y_, this->ball_time_,
                     this->ball_speed_, this->ball_max_bounce_angle_,
                     this->ball_radius_);
    // return this->draw_()
  }


  StateType step(const int player1_action, int player2_action) {
    // Ignore given action for player 2 if 'best player' option is selected
    if (this->play_as_best_player_) {
      const int player_id = 1;
      player2_action = this->get_follow_ball_action(player_id);
    }
    // Ceil player 1 position
    this->player1_y_ += player1_action;
    this->player1_y_ = std::min(this->player1_y_, this->screen_h_ - this->bar_h_);
    this->player1_y_ = std::max(this->player1_y_, 0);
    // Ceil player 2 position
    this->player2_y_ += player2_action;
    this->player2_y_ = std::min(this->player2_y_, this->screen_h_ - this->bar_h_);
    this->player2_y_ = std::max(this->player2_y_, 0);
    PlayerStatusType status = this->ball_.update_pos(this->player1_y_, this->player2_y_,
                                    this->wall_player1_, this->wall_player2_,
                                    this->bar_h_, this->screen_h_);
    int reward_player1 = 0;
    int reward_player2 = 0;
    if (status.player1 == kHitBall) {
      reward_player1 = 1;
    }
    if (status.player1 == kWon) {
      reward_player1 = 1;
      this->player1_score_ += 1;
    }
    else if (status.player1 == kLost) {
      reward_player1 = -1;
    }
    if (status.player2 == kHitBall) {
      reward_player2 = 1;
    }
    if (status.player2 == kWon) {
      reward_player2 = 1;
      this->player2_score_ += 1;
    }
    else if (status.player2 == kLost) {
      reward_player2 = -1;
    }
    // Randomize ball x-direction when restarting
    if (status.player1 == kWon || status.player2 == kWon) {
      if (xorrand() % 2 == 0) {
        this->ball_.vx_ *= -1;
      } else {
        this->ball_.vx_ *= 1;
      }
      if (xorrand() % 2 == 0) {
        this->ball_.vy_ = -1;
      } else {
        this->ball_.vy_ = 1;
      }
    }
    StateType state;
    const bool draw_score = false;
    // state.observation = this->draw(draw_score);
    state.reward_player1;
    state.reward_player2;
    state.info = this->screen_img_;
    if (this->player1_score_ >= 10 || this->player2_score_ >= 10) {
      state.done = true;
      this->reset();
    } else {
      state.done = false;
    }
    return state;
  }

  // Screen parameters
  int screen_h_;
  int screen_w_;
  int score_h_;
  int score_w_;
  // Ball parameters
  int ball_x_;
  int ball_y_;
  T ball_speed_;
  T ball_radius_;
  T ball_max_bounce_angle_;
  T ball_time_;
  BallV2<T> ball_;
  // Bars parameters
  int bar_w_;
  int bar_h_;
  int wall_player1_;
  int wall_player2_;
  // Player parameters
  int player1_y_;
  int player2_y_;
  int player1_score_;
  int player2_score_;
  ImageBuffer<H, W, T> screen_img_;
  bool play_as_best_player_;
};


#endif // end GAME_GAME_H_