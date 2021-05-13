#include "tb/test_hdmi.h"

int main(int argc, char const *argv[]) {
  // AxiStreamRGB tpgStream, maskStream, paintedStream;

  std::cout << "Starting HDMI application.\n";

  // IplImage* testImage = cvLoadImage(INPUT_IMAGE);
  // IplImage* maskImage = cvLoadImage(MASK_IMAGE);

  // if (testImage == nullptr || maskImage == nullptr) { // check if we succeeded
  //   std::cerr << "ERROR. Unable to open images. Exiting.\n";
  //   return -1;
  // }
  // std::cout << "Images read: " << testImage->width << "x" << testImage->height << "\n";
  // IplImage* paintedImage = cvCreateImage(cvGetSize(testImage), testImage->depth, testImage->nChannels);

  // IplImage2AXIvideo(testImage, tpgStream);
  // IplImage2AXIvideo(maskImage, maskStream);
  // std::cout << "Calling painter.\n";
  // painter(tpgStream, maskStream, paintedStream);

  // AXIvideo2IplImage(paintedStream, paintedImage);

  // std::cout << "Saving image.\n";
  // cvSaveImage(OUTPUT_IMAGE, paintedImage);
  // cvReleaseImage(&testImage);
  // cvReleaseImage(&maskImage);
  // cvReleaseImage(&paintedImage);

  // Test pong
  // cv::Mat image1;
  // IplImage* image2;
  // image2 = cvCreateImage(cvSize(image1.cols,image1.rows),8,3);
  // IplImage ipltemp=image1;
  // cvCopy(&ipltemp,image2);
  // 
  // cv::Mat mat = grHistogram(MAX_HEIGHT, MAX_WIDTH, CV_8UC3, Scalar(0, 0, 0));
  // cvCopy(&ipltemp, image2);

#if 0
  AxiStreamRGB pongStream;
  unsigned int player1_score = 0;
  unsigned int player2_score = 0;
  IplImage* pongImage = cvCreateImage(cvSize(MAX_WIDTH, MAX_HEIGHT), 8, 3);
  IplImage* hlsImage = cvCreateImage(cvSize(MAX_WIDTH, MAX_HEIGHT), 8, 3);
  cvSet(pongImage, cvScalar(0, 0, 0));
  const int steps = 8;
  for (int i = 0; i < steps; ++i) {
    std::cout << "Image n." << i << ")\n";
    const bool player_1_move_right = false; // int(rand()) % 1 == 0 ? true : false;
    const bool player_1_move_left = true; // player_1_move_right ? false : int(rand()) % 1;
    const bool player_2_move_right = false; // int(rand()) % 1 == 0 ? true : false;
    const bool player_2_move_left = true; // player_2_move_right ? false : int(rand()) % 1;
    std::cout << "Player 1: moves " << (player_1_move_left ? "L" : (player_1_move_right ? "R" : "0")) << "\n";
    std::cout << "Player 2: moves " << (player_2_move_left ? "L" : (player_2_move_right ? "R" : "0")) << "\n";
    pong(player_1_move_left, player_1_move_right, player_2_move_left,
      player_2_move_right, player1_score, player2_score, pongStream);
    AXIvideo2IplImage(pongStream, pongImage);
    std::string filename("C:/Users/ste/phd/pynq_projects/hdmi_noise/data/pong_" + std::to_string(i) + ".bmp");
    cvSaveImage(filename.c_str(), pongImage);
    std::cout << "\n";
  }
  cvReleaseImage(&hlsImage);
  cvReleaseImage(&pongImage);
#endif
  
  return 0;
}