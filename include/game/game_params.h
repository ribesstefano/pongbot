#ifndef GAME_GAME_PARAMS_H_
#define GAME_GAME_PARAMS_H_

#include "hls_stream.h"
#include "hls_video.h"
#include "ap_int.h"

#define WHITE_PIXEL 255
#define BLACK_PIXEL 0

#define MAX_WIDTH  800
#define MAX_HEIGHT 600

#define INPUT_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/windows.bmp"
#define MASK_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/mask.bmp"
#define OUTPUT_IMAGE "C:/Users/ste/phd/pynq_projects/hdmi_noise/data/output.bmp"

typedef hls::stream<ap_axiu<24, 1, 1, 1> > AxiStreamRGB;
typedef hls::Scalar<3, unsigned char> PixelRGB;
typedef hls::Window<MAX_HEIGHT, MAX_WIDTH, PixelRGB> WindowRGB;
typedef hls::Window<1, MAX_WIDTH, PixelRGB> PlayerWindowRGB;
typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC3> ImageRGB;

static const int kWhitePixel = 255;
static const int kBlackPixel = 0;

template <int H, int W, typename T>
class ImageBuffer {
public:
  ImageBuffer(): rows(H), cols(W), size(H * W) {
    Init_Image:
    for (int i = 0; i < H; ++i) {
      for (int j = 0; j < W; ++j) {
#pragma HLS PIPELINE II=1
        this->_buffer[i * W + j] = BLACK_PIXEL;
        // this->_buffer[i][j] = BLACK_PIXEL;
      }
    }
  };

  ~ImageBuffer() {};

  T* operator[](const int i) {
    return reinterpret_cast<T*>(&(this->_buffer[i * W]));
  }

  int rows;
  int cols;
  int size;
  T _buffer[H * W];
};

typedef enum PlayerStatus_ {kHitBall, kWon, kLost, kNoChanges} PlayerStatus;
typedef struct PlayerStatusType {
  PlayerStatus player1;
  PlayerStatus player2;
} PlayerStatusType;

typedef hls::Mat<MAX_HEIGHT, MAX_WIDTH, HLS_8UC1> CvImage;
typedef ImageBuffer<MAX_HEIGHT, MAX_WIDTH, unsigned char> Image;

typedef ap_fixed<8, 6> GameRealType;

#endif // end GAME_GAME_PARAMS_H_