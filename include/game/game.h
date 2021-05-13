#ifndef GAME_GAME_H_
#define GAME_GAME_H_

#include <hls_stream.h>
#include <hls_video.h> // This must be included in the source/syn files only

#include "game/point.h"
#include "game/ball.h"

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

#endif // end GAME_GAME_H_