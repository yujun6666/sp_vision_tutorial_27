#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{
    cv::Mat image = cv::imread("../assets/demo.jpg");
    if (image.empty()) {
        std::cerr << "Cannot read ../assets/demo.jpg\n";
        return 1;
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    if (!cv::imwrite("gray.jpg", gray)) {
        std::cerr << "Cannot write gray.jpg\n";
        return 1;
    }

    cv::circle(gray, cv::Point(gray.cols / 2, gray.rows / 2),
               30, cv::Scalar(255), 2);
    cv::imshow("gray", gray);
    cv::waitKey(0);
    return 0;
}