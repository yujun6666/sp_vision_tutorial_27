#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <opencv2/opencv.hpp>

#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "tools/img_tools.hpp"

int main()
{
  try {
    const std::string config_path = "./configs/yolo.yaml";

    // Load the model once. false disables the detector's extra debug drawing.
    auto_aim::YOLO yolo(config_path, false);
    io::Camera camera(config_path);

    cv::namedWindow("Armor detection", cv::WINDOW_NORMAL);
    int frame_count = 0;
    int empty_frames = 0;

    while (true) {
      cv::Mat img = camera.read();

      if (!img.empty()) {
        empty_frames = 0;
        const auto armors = yolo.detect(img, frame_count++);

        for (const auto & armor : armors) {
          // draw_points already connects the last point back to the first.
          tools::draw_points(img, armor.points, cv::Scalar(0, 255, 0), 2);
        }

        cv::imshow("Armor detection", img);
      } else if (++empty_frames >= 5) {
        throw std::runtime_error("Camera returned no image for 5 consecutive reads; check USB connection and camera settings.");
      }

      // Poll the keyboard even when a frame times out.
      const int key = cv::waitKey(1);
      if (key == 'q' || key == 'Q' || key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
  } catch (const std::exception & error) {
    std::cerr << "Armor detection failed: " << error.what() << '\n'
              << "Run this program from lecture2/homework, using ./build/main.\n";
    return 1;
  }
}
