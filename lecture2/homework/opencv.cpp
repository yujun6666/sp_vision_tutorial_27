#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <opencv2/opencv.hpp>

#include "io/camera.hpp"
#include "tasks/apriltag_detector.hpp"
#include "tools/img_tools.hpp"

int main()
{
  try {
    const std::string config_path = "./configs/yolo.yaml";
    auto_charge::AprilTagDetector detector(config_path);
    io::Camera camera(config_path);
    cv::namedWindow("AprilTag detection", cv::WINDOW_NORMAL);
    int empty_frames = 0;

    while (true) {
      cv::Mat img = camera.read();

      if (!img.empty()) {
        empty_frames = 0;
        // The provided detector finds configured AprilTags, not a plain logo.
        const auto tags = detector.detect(img);
        for (const auto & tag : tags) {
          tools::draw_points(img, tag.corners, cv::Scalar(0, 255, 0), 2);
          tools::draw_text(
            img, "ID " + std::to_string(tag.id), tag.center,
            cv::Scalar(0, 255, 0), 0.8, 2);
        }

        cv::imshow("AprilTag detection", img);
      } else if (++empty_frames >= 5) {
        throw std::runtime_error("Camera returned no image for 5 consecutive reads; check USB connection and camera settings.");
      }

      const int key = cv::waitKey(1);
      if (key == 'q' || key == 'Q' || key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
  } catch (const std::exception & error) {
    std::cerr << "AprilTag detection failed: " << error.what() << '\n'
              << "Run this program from lecture2/homework, using ./build/opencv.\n";
    return 1;
  }
}
