#ifndef HDMI_OUT_H_
#define HDMI_OUT_H_

#include <hls_stream.h>
#include <hls_video.h> // This must be included in the source files.
// #include <hls_opencv.h> // This must be included in the testbench ONLY.
#include "point.h"

#if 0
#define MAX_WIDTH  800
#define MAX_HEIGHT 600

#define INPUT_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/windows.bmp"
#define MASK_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/mask.bmp"
#define OUTPUT_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/output.bmp"

typedef hls::stream<ap_axiu<24, 1, 1, 1> > AxiStreamRGB;
typedef hls::Scalar<3, unsigned char> PixelRGB;
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC3> ImageRGB;
typedef hls::Window<MAX_HEIGHT, MAX_WIDTH, PixelRGB> WindowRGB;
typedef hls::Window<1, MAX_WIDTH, PixelRGB> PlayerWindowRGB;

void painter(AxiStreamRGB &tpg, AxiStreamRGB &mask, AxiStreamRGB &painted);

void pong(const bool player1_move_left, const bool player1_move_right,
  const bool player2_move_left, const bool player2_move_right,
  unsigned int &player1_score, unsigned int &player2_score,
  AxiStreamRGB &outStream);

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


class BallV2 {
public:
  BallV2(const int x, const int y, const float time = 1);
  ~BallV2();

  int update_pos(const int player1_pos, const int player2_pos,
      const int wall_player1, const int wall_player2, const int bar_length) {
    // Update position THEN change velocity and position.
    // NOTE: time is the time since last frame, in milliseconds (therefore
    // the velocities are in pixels per millisecond).
    this->_x += this->_vx * this->_time;
    this->_y += this->_vy * this->_time;
    this->check_height_collision();
    const int game_score = this->check_player_collision(player1_pos,
      player2_pos, wall_player1, wall_player2, bar_length);
    return game_score;
  }

private:
  void check_height_collision() {
    if (this->_y - this->_radius <= 0) {
      this->_vy = -this->_vy;
      this->_y = this->_radius;
    }
    if (this->_y + this->_radius >= screen_heigth) {
      this->_vy = -this->_vy;
      this->_y = screen_heigth - this->_radius;
    }
  }

  void update_velocity(const int collision_y, const int player_pos,
      const int bar_length) {
    // The collision_y: y-coord where the ball collided
    const float rel_intersect = player_pos + bar_length / 2 - collision_y;
    const float norm_rel_intersect = rel_intersect / (bar_length / 2);
    const float bounce_angle = norm_rel_intersect_y * this->_max_bounce_angle;
    // NOTE: vx should never go to zero, otherwise the ball will go in a
    // vertical direction.
    this->_vx = std::max(this->_speed * cos(bounce_angle), this->_radius / 2);
    this->_vy = this->_speed * -sin(bounce_angle);
  }

  int check_player_collision(const int player1_pos, const int player2_pos,
    const int wall_player1, const int wall_player2, const int bar_length) {
    if (this->_x - this->_radius <= wall_player1) {
      if (player1_pos < this->_y < player1_pos + bar_length) {
        this->_update_velocity(this->_y, player1_pos, bar_length);
        this->_x = wall_player1 + this->_radius + 1;
        return 0; // {'player1':'hit ball', 'player2':'no changes'}
      } else {
        this->_x = this->_init_x;
        this->_y = this->_init_y;
        return 0; // {'player1':'won', 'player2':'lost'}
      }
    }
    if (this->_x + this->_radius >= wall_player2) {
      if (player2_pos < this->_y < player2_pos + bar_length) {
        this->_update_velocity(this->_y, player2_pos, bar_length);
        this->_x = wall_player2 - this->_radius - 1;
        this->_vx = -this->_vx; // TODO: Understand why only on this side
        return 0; // {'player1':'no changes', 'player2':'hit ball'}
      } else {
        this->_x = this->_init_x;
        this->_y = this->_init_y;
        return 0; // {'player1':'lost', 'player2':'won'}
      }
    }
    return 0; // {'player1':'no changes', 'player2':'no changes'}
  }

  int _init_x;
  int _init_y;
  int _x;
  int _y;
  int _speed;
  int _time;
  float _vx;
  float _vy;
  float _max_bounce_angle;
  int _radius;
};
#endif



#endif