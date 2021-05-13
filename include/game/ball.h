#ifndef BALL_H_
#define BALL_H_

#include "game/point.h"
#include <iostream>

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

#endif // end BALL_H_