#ifndef GAME_BALL_H_
#define GAME_BALL_H_

#include <iostream>
#include "hls_math.h"
#include "ap_int.h"

#include "game/game_params.h"
#include "game/game_utils.h"

static const float Pi = 3.14159265358979323846;
static const float kRidiantScaler = Pi / 180.0;

template <typename T>
class Ball {
private:
  T init_v() {
    T v;
    if (xorrand() % 2 == 0) {
      v = this->_speed;
    } else {
      v = -this->_speed;
    }
    // T v = this->_speed; // np.random.randint(-this->speed, this->speed);
    // while (v == 0) {
    //   v = this->_speed + 1; // np.random.randint(-this->speed, this->speed);
    // }
    // TODO: Find better approximation for sqrt(2).
    return v * T(1.414); // sqrt(2);
  }

public:
  Ball(const int x, const int y, const T time, const T speed,
    const int max_bounce_angle, const int radius) {
    this->_x = x;
    this->_y = y;
    this->_speed = speed;
    this->_vx = this->init_v();
    this->_vy = this->init_v();
    this->_init_x = x;
    this->_init_y = y;
    this->_time = time;
    this->_max_bounce_angle = max_bounce_angle * T(kRidiantScaler); // convert to radiants;
    this->_radius = radius;
  }
  ~Ball() {}

  void reset_pos() {
    this->_x = this->_init_x;
    this->_y = this->_init_y;
    this->_vx = this->init_v();
    this->_vy = this->init_v();
  }

  void update_velocity(const int collision_y, const int player_y,
      const int bar_heigth) {
    // The collision_y: y-coord where the ball collided
    T rel_intersect_y = player_y - collision_y + bar_heigth / 2; // In [-bar/2, bar/2] interval
    rel_intersect_y = rel_intersect_y  / (bar_heigth / 2); // Normalize, now in [-1, 1] interval
    // NOTE: The angle will be at its maximum when collision_y is either 
    // equal to 'player_y' or 'player_y + bar_heigth'
    T bounce_angle = rel_intersect_y * this->_max_bounce_angle;
    // NOTE: vx should never go to zero, otherwise the ball will go in a
    // vertical direction (90 degrees).
#define Q15 (T(1.0) / (T)((1 << 15) - 1))
    T _vxtmp = this->_speed * cos1(bounce_angle * T(0.159155)) * Q15;
    T change_direction = this->_vx > 0 ? -1 : 1;
    this->_vx = _vxtmp != 0 ? T(_vxtmp * change_direction) : T(-this->_vx);
    this->_vy = this->_speed * -sin1(bounce_angle  * T(0.159155)) * Q15;
  }

  PlayerStatusType check_player_collision(const int player1_y, const int player2_y,
      const int wall_player1, const int wall_player2, const int bar_heigth) {
#pragma HLS PIPELINE II=1
    PlayerStatusType players_status;
    players_status.player1 = kNoChanges;
    players_status.player2 = kNoChanges;
    if (this->_x - this->_radius <= wall_player1) {
      if (player1_y < this->_y && this->_y < player1_y + bar_heigth) {
        this->update_velocity(this->_y, player1_y, bar_heigth);
        this->_x = wall_player1 + this->_radius;
        players_status.player1 = kHitBall;
        players_status.player2 = kNoChanges;
        // return {'player1':'hit ball', 'player2':'no changes'}
      } else {
        this->reset_pos();
        players_status.player1 = kLost;
        players_status.player2 = kWon;
        // return {'player1':'lost', 'player2':'won'}
      }
    }
    if (this->_x + this->_radius >= wall_player2) {
      if (player2_y < this->_y && this->_y < player2_y + bar_heigth) {
        this->update_velocity(this->_y, player2_y, bar_heigth);
        this->_x = wall_player2 - this->_radius;
        players_status.player1 = kNoChanges;
        players_status.player2 = kHitBall;
        // return {'player1':'no changes', 'player2':'hit ball'}
      } else {
        this->reset_pos();
        players_status.player1 = kWon;
        players_status.player2 = kLost;
        // return {'player1':'won', 'player2':'lost'}
      }
    }
    return players_status;
  }

  PlayerStatusType update_pos(const int player1_y, const int player2_y,
      const int wall_player1, const int wall_player2,
      const int bar_heigth, const int screen_heigth) {
    // '''
    // Update position THEN change velocity and position.
    // '''
    // NOTE: time is the time since last frame, in milliseconds (therefore
    // the velocities are in pixels per millisecond).
    this->_x += int(this->_vx * this->_time);
    this->_y += int(this->_vy * this->_time);
    this->check_height_collision(screen_heigth);
    return this->check_player_collision(player1_y, player2_y, wall_player1,
      wall_player2, bar_heigth);
  }

  void check_height_collision(const int screen_heigth) {
#pragma HLS PIPELINE II=1
    if (this->_y - this->_radius <= 0) {
        this->_vy = -this->_vy;
        this->_y = this->_radius;
    }
    if (this->_y + this->_radius >= screen_heigth) {
        this->_vy = -this->_vy;
        this->_y = screen_heigth - this->_radius;
    }
  }

  template <int H, int W, typename I>
  void draw_square(ImageBuffer<H, W, I> &img) {
    const int x_low = std::max(0, this->_x - this->_radius + 1);
    const int x_high = std::min(img.cols - 1, this->_x + this->_radius);
    const int y_low = std::max(0, this->_y - this->_radius + 1);
    const int y_high = std::min(img.rows - 1, this->_y + this->_radius);
    for (int i = y_low; i <= y_high; ++i) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=2*this->_radius
      for (int j = x_low; j <= x_high; ++j) {
#pragma HLS LOOP_TRIPCOUNT min=0 max=2*this->_radius
#pragma HLS PIPELINE II=1
        img[i][j] = WHITE_PIXEL;
      }
    }
  }

  template <int H, int W, typename I>
  void draw(const int screen_h, const int screen_w, ImageBuffer<H, W, I> &img) {
    this->draw_square(img);
  }
  
  int _x;
  int _y;
  int _init_x;
  int _init_y;
  T _speed;
  T _vx;
  T _vy;
  T _time;
  T _max_bounce_angle;
  int _radius;
};

#endif // end GAME_BALL_H_