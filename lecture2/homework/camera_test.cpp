#include <exception>
#include <iostream>
#include <stdexcept>

#include <opencv2/opencv.hpp>

#include "io/camera.hpp"

int main()
{
  try {
    io::Camera camera("./configs/yolo.yaml");
    cv::namedWindow("Camera test", cv::WINDOW_NORMAL);
    int empty_frames = 0;

    while (true) {
      cv::Mat img = camera.read();
      if (!img.empty()) {
        empty_frames = 0;
        cv::imshow("Camera test", img);
      } else if (++empty_frames >= 5) {
        throw std::runtime_error("Camera returned no image for 5 consecutive reads; check USB connection and camera settings.");
      }

      const int key = cv::waitKey(1);
      if (key == 'q' || key == 'Q' || key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
  } catch (const std::exception & error) {
    std::cerr << "Camera test failed: " << error.what() << '\n'
              << "Run from lecture2/homework and connect the USB camera to Ubuntu.\n";
    return 1;
  }
}
