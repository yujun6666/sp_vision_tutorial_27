#ifndef IO__CAMERA_HPP
#define IO__CAMERA_HPP

#include <opencv2/core.hpp>
#include <string>

namespace io
{

class Camera
{
public:
  explicit Camera(const std::string & config_path);
  ~Camera();
  cv::Mat read();

private:
  // 一个相机句柄只能由一个对象负责关闭，禁止复制。
  Camera(const Camera &) = delete;
  Camera & operator=(const Camera &) = delete;
  void close() noexcept;

  void * handle_ = nullptr;
  bool opened_ = false;
  bool grabbing_ = false;
};

}  // namespace io

#endif  // IO__CAMERA_HPP
