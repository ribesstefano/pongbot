#ifndef GAME_BALL_H_
#define GAME_BALL_H_

#include <iostream>
#include "hls_math.h"
#include "ap_int.h"

#include "game_params.h"
#include "game_utils.h"
#include "point.h"

static const float Pi = 3.14159265358979323846;
static const float kRidiantScaler = Pi / 180.0;

template <typename T>
class BallV2 {
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
  BallV2(const int x, const int y, const T time, const T speed,
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
  ~BallV2() {}

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

template <int RadiusInPixes = 2>
class Ball {
public:
  Ball(const int screen_height, const int screen_width, const int bar_height,
    const int bar_width, int (&tail)[2], int (&arrow)[2]):
      tail_(Point2D(tail, 0, 0)),
      // arrow_(Point2D(arrow, -RadiusInPixes, -20)),
      arrow_(Point2D(arrow, 0, -20)),
      screen_height_(screen_height), screen_width_(screen_width),
      bar_height_(bar_height), bar_width_(bar_width),
      radius_pow_(RadiusInPixes * RadiusInPixes) {}
  ~Ball() {};

  int update_ball_position(const int player1_y, const int player2_y) {
#pragma HLS PIPELINE II=1
    int losing_player_id = 0;
    const int x_tail_old = tail_.get_x();
    const int y_tail_old = tail_.get_y();

    const int x_arrow_old = arrow_.get_x();
    const int y_arrow_old = arrow_.get_y();
    tail_.set_x(arrow_.get_x());
    tail_.set_y(arrow_.get_y());
    arrow_.set_x(arrow_.get_x() - (x_tail_old - arrow_.get_x()));
    arrow_.set_y(arrow_.get_y() - (y_tail_old - arrow_.get_y()));

    // NOTE: The center is in the middle.
    if (tail_.get_x() <= -screen_width_ / 2 + RadiusInPixes) {
      this->mirror(false, true, screen_width_ / 2); // mirror w.r.t left wall
    }
    if (tail_.get_x() >= screen_width_ / 2 - RadiusInPixes) {
      this->mirror(false, false, screen_width_ / 2);  // mirror w.r.t right wall
    }
    if (tail_.get_y() <= -screen_height_ / 2 + bar_height_ + RadiusInPixes) {
      // Check player 1 position
      if (tail_.get_x() >= player1_y &&
          tail_.get_x() <= player1_y + bar_width_) {
        this->mirror(true, true, screen_height_ / 2 - bar_height_);
      } else {
#ifndef __SYNTHESIS__
        std::cout << "\t\t\t\tGAME OVER! Player 1 lost.\n";
#endif
        losing_player_id = 1;
      }
    }
    if (tail_.get_y() >= screen_height_ / 2 - bar_height_ - RadiusInPixes) {
      // Check player 2 position
      if (tail_.get_x() >= player2_y &&
          tail_.get_x() <= player2_y + bar_width_) {
        this->mirror(true, false, screen_height_ / 2 - bar_height_);
      } else {
#ifndef __SYNTHESIS__
        std::cout << "\t\t\t\tGAME OVER! Player 2 lost.\n";
#endif
        losing_player_id = 2;
      }
    }
#ifndef __SYNTHESIS__
    std::cout << "Ball at position: (" << tail_.get_x() << ", " << tail_.get_y()
      << ") -> (" << arrow_.get_x() << ", " << arrow_.get_y() << ")\n";
#endif
    return losing_player_id ;
  }

  void get_tail(int &x, int &y) {
    x = tail_.get_x();
    y = tail_.get_y();
  }

  int get_x() {
#pragma HLS INLINE
    return tail_.get_x();
  }

  int get_y() {
#pragma HLS INLINE
    return tail_.get_y();
  }

  bool is_ball_pixel(const int point_x, const int point_y,
      const bool draw_circle = false) {
#pragma HLS INLINE
#pragma HLS PIPELINE II=1
    const int ball_x = tail_.get_x() + screen_width_ / 2;
    const int ball_y = tail_.get_y() + screen_height_ / 2;
    if (draw_circle) {
      // Draw the ball as a circle
      // (x1 - x_center)**2 + (y1 - y_center)**2 <= r**2
      const int x_diff = (point_x - ball_x) * (point_x - ball_x);
      const int y_diff = (point_y - ball_y) * (point_y - ball_y);
      if (x_diff + y_diff <= radius_pow_) {
        return true;
      } else {
        return false;
      }
    } else {
      // Draw the ball as a square
      // Given a rectangle with points (x1,y1) and (x2,y2) and assuming
      // x1 < x2 and y1 < y2 (if not, you can just swap them), a point (x,y)
      // is within that rectangle if x1 < x < x2 and y1 < y < y2.
      int x_low = ball_x - RadiusInPixes;
      int x_high = ball_x + RadiusInPixes;
      int y_low = ball_y - RadiusInPixes;
      int y_high = ball_y + RadiusInPixes;
      if (x_low <= point_x && point_x <= x_high &&
          y_low <= point_y && point_y <= y_high) {
        return true;
      } else {
        return false;
      }
    }
  }

  void reset_ball_position() {
#pragma HLS INLINE
    tail_.set_x(0);
    arrow_.set_x(-RadiusInPixes);
    tail_.set_y(0);
    arrow_.set_y(-20);
  }

private:

  int abs(int a) {
#pragma HLS INLINE
    return a < 0 ? -a : a;
  }

  void mirror(const bool mirror_vertical, const bool mirror_negative,
      const int wall) {
#pragma HLS INLINE
#pragma HLS PIPELINE II=1
    int coord_idx = 0; // coordinate index, either x == 0 or y == 1
    int direction = 1;
    if (mirror_vertical) {
      coord_idx = 1;
    }
    if (mirror_negative) { // The left and bottom sides
      direction = -1;
    }
    arrow_[coord_idx] = direction * (wall - this->abs(arrow_[coord_idx] - tail_[coord_idx]) - RadiusInPixes);
    tail_[coord_idx] = direction * (wall - RadiusInPixes);
  }

  Point2D tail_; // The first element is the arrow of the velocity vector.
  Point2D arrow_; // The first element is the arrow of the velocity vector.
  const int radius_pow_;
  const int screen_height_;
  const int screen_width_;
  const int bar_height_;
  const int bar_width_;
};

#endif // end GAME_BALL_H_