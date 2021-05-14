#ifndef GAME_BALL_H_
#define GAME_BALL_H_

#include <iostream>
#include "hls_math.h"
#include "game_params.h"
#include "point.h"

static const float Pi = 3.14159265358979323846;

template <typename T>
class BallV2 {
private:
  T init_v() {
    T v = this->speed_; // np.random.randint(-this->speed, this->speed);
    while (v == 0) {
      v = this->speed_ + 1; // np.random.randint(-this->speed, this->speed);
    }
    return v * sqrt(2);
  }

public:
  BallV2(const T x, const T y, const T time, const T speed,
    const T max_bounce_angle, const T radius) {
    this->x_ = x;
    this->y_ = y;
    this->speed_ = speed;
    this->vx_ = this->init_v();
    this->vy_ = this->init_v();
    this->init_x_ = x;
    this->init_y_ = y;
    this->time_ = time;
    this->max_bounce_angle_ = max_bounce_angle * Pi / 180.0; // convert to radiants;
    this->radius_ = radius;
  }
  ~BallV2() {}

  void reset_pos() {
    this->x_ = this->init_x_;
    this->y_ = this->init_y_;
    this->vx_ = this->init_v();
    this->vy_ = this->init_v();
  }

  void update_velocity(const T collision_y, const T player_y, const T bar_heigth) {
      // The collision_y: y-coord where the ball collided
      T rel_intersect_y = player_y - collision_y + bar_heigth / 2; // In [-bar/2, bar/2] interval
      rel_intersect_y = rel_intersect_y  / (bar_heigth / 2); // Normalize, now in [-1, 1] interval
      //  NOTE: The angle will be at its maximum when collision_y is either 
      // equal to 'player_y' or 'player_y + bar_heigth'
      T bounce_angle = rel_intersect_y * this->max_bounce_angle_;
      // NOTE: vx should never go to zero, otherwise the ball will go in a
      // vertical direction (90 degrees).
      T vx_tmp = this->speed_ * cos(bounce_angle);
      T change_direction = this->vx_ > 0 ? -1 : 1;
      this->vx_ = vx_tmp != 0 ? vx_tmp * change_direction : -this->vx_;
      this->vy_ = this->speed_ * -sin(bounce_angle);
  }

  PlayerStatusType check_player_collision(const T player1_y, const T player2_y,
      const T wall_player1, const T wall_player2, const T bar_heigth) {
    PlayerStatusType players_status;
    players_status.player1 = kNoChanges;
    players_status.player2 = kNoChanges;
    if (this->x_ - this->radius_ <= wall_player1) {
      if (player1_y < this->y_ < player1_y + bar_heigth) {
        this->update_velocity(this->y_, player1_y, bar_heigth);
        this->x_ = wall_player1 + this->radius_;
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
    if (this->x_ + this->radius_ >= wall_player2) {
      if (player2_y < this->y_ < player2_y + bar_heigth) {
        this->update_velocity(this->y_, player2_y, bar_heigth);
        this->x_ = wall_player2 - this->radius_;
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

  PlayerStatusType update_pos(const T player1_y, const T player2_y,
      const T wall_player1, const T wall_player2,
      const T bar_heigth, const T screen_heigth) {
    // '''
    // Update position THEN change velocity and position.
    // '''
    // NOTE: time is the time since last frame, in milliseconds (therefore
    // the velocities are in pixels per millisecond).
    this->x_ += this->vx_ * this->time_;
    this->y_ += this->vy_ * this->time_;
    this->check_height_collision(screen_heigth);
    return this->check_player_collision(player1_y, player2_y,
                                             wall_player1, wall_player2,
                                             bar_heigth);
  }

  void check_height_collision(const T screen_heigth) {
    if (this->y_ - this->radius_ <= 0) {
        this->vy_ = -this->vy_;
        this->y_ = this->radius_;
    }
    if (this->y_ + this->radius_ >= screen_heigth) {
        this->vy_ = -this->vy_;
        this->y_ = screen_heigth - this->radius_;
    }
  }

  template <int H, int W, typename I>
  void draw_square(ImageBuffer<H, W, I> &img) {
#pragma HLS PIPELINE II=1
    for (int i = 0; i < img.rows; ++i) {
      for (int j = 0; j < img.cols; ++j) {
        if (i > this->x_ - this->radius_ && i < this->x_ + this->radius_) {
          if (j > this->y_ - this->radius_ && j < this->y_ + this->radius_) {
            img[i][j] = WHITE_PIXEL;
            // img.buffer_[i][j] = WHITE_PIXEL;
          }
        }
      }
    }
  }

  template <int H, int W, typename I>
  void draw(ImageBuffer<H, W, I> &img, const T screen_h, const T screen_w) {
    this->draw_square(img);
  }
  
  T x_;
  T y_;
  T speed_;
  T vx_;
  T vy_;
  T init_x_;
  T init_y_;
  T time_;
  T max_bounce_angle_;
  T radius_;
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