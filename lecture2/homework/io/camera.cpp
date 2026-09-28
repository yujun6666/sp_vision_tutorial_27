#include "camera.hpp"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "hikrobot/include/MvCameraControl.h"

namespace
{

std::string error_message(const char * operation, int code)
{
  std::ostringstream message;
  message << operation << " failed, error code: 0x" << std::hex
          << static_cast<unsigned int>(code);
  return message.str();
}

void check(int code, const char * operation)
{
  if (code != MV_OK) throw std::runtime_error(error_message(operation, code));
}

void warn(int code, const char * operation)
{
  if (code != MV_OK) std::cerr << "Warning: " << error_message(operation, code) << '\n';
}

}  // namespace

namespace io
{

Camera::Camera(const std::string & config_path)
{
  // 对应 configs/yolo.yaml 中新增的 camera 配置。
  const auto config = YAML::LoadFile(config_path)["camera"];
  const float exposure_us = config["exposure_us"].as<float>();
  const float gain = config["gain"].as<float>();
  const float frame_rate = config["frame_rate"].as<float>();
  if (!(exposure_us > 0) || !(gain >= 0) || !(frame_rate > 0)) {
    throw std::runtime_error("Invalid camera settings: check exposure_us, gain and frame_rate.");
  }

  MV_CC_DEVICE_INFO_LIST device_list{};
  check(MV_CC_EnumDevices(MV_USB_DEVICE, &device_list), "MV_CC_EnumDevices");
  if (device_list.nDeviceNum == 0) {
    throw std::runtime_error("No USB camera found. Connect the camera to the Ubuntu VM first.");
  }

  check(MV_CC_CreateHandle(&handle_, device_list.pDeviceInfo[0]), "MV_CC_CreateHandle");
  try {
    check(MV_CC_OpenDevice(handle_), "MV_CC_OpenDevice");
    opened_ = true;

    // 连续取图，不等待外部触发信号。
    check(MV_CC_SetEnumValue(handle_, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS), "AcquisitionMode");
    check(MV_CC_SetEnumValue(handle_, "TriggerMode", MV_TRIGGER_MODE_OFF), "TriggerMode");
    check(MV_CC_SetEnumValue(handle_, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF), "ExposureAuto");
    check(MV_CC_SetEnumValue(handle_, "GainAuto", MV_GAIN_MODE_OFF), "GainAuto");
    check(MV_CC_SetFloatValue(handle_, "ExposureTime", exposure_us), "ExposureTime");
    check(MV_CC_SetFloatValue(handle_, "Gain", gain), "Gain");

    // 部分型号不支持自动白平衡或帧率设置，失败时提示并继续。
    warn(MV_CC_SetEnumValue(handle_, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_CONTINUOUS),
         "BalanceWhiteAuto");
    warn(MV_CC_SetFrameRate(handle_, frame_rate), "FrameRate");

    check(MV_CC_StartGrabbing(handle_), "MV_CC_StartGrabbing");
    grabbing_ = true;
  } catch (...) {
    // 构造失败时析构函数不会被调用，需要在这里释放已经取得的资源。
    close();
    throw;
  }
}

cv::Mat Camera::read()
{
  MV_FRAME_OUT raw{};
  const int result = MV_CC_GetImageBuffer(handle_, &raw, 1000);
  if (result == MV_E_NODATA) return {};  // 超时没有图像，交给调用者决定是否重试。
  check(result, "MV_CC_GetImageBuffer");

  cv::Mat image;
  try {
    if (raw.pBufAddr == nullptr || raw.stFrameInfo.nWidth == 0 || raw.stFrameInfo.nHeight == 0) {
      throw std::runtime_error("Camera returned an invalid frame.");
    }

    // 自己分配三通道图像，避免归还 SDK 缓冲区后图像数据失效。
    image.create(raw.stFrameInfo.nHeight, raw.stFrameInfo.nWidth, CV_8UC3);
    MV_CC_PIXEL_CONVERT_PARAM conversion{};
    conversion.nWidth = raw.stFrameInfo.nWidth;
    conversion.nHeight = raw.stFrameInfo.nHeight;
    conversion.pSrcData = raw.pBufAddr;
    conversion.nSrcDataLen = raw.stFrameInfo.nFrameLen;
    conversion.enSrcPixelType = raw.stFrameInfo.enPixelType;
    conversion.pDstBuffer = image.data;
    conversion.nDstBufferSize = static_cast<unsigned int>(image.total() * image.elemSize());
    conversion.enDstPixelType = PixelType_Gvsp_BGR8_Packed;

    check(MV_CC_ConvertPixelType(handle_, &conversion), "MV_CC_ConvertPixelType");
  } catch (...) {
    MV_CC_FreeImageBuffer(handle_, &raw);
    throw;
  }

  check(MV_CC_FreeImageBuffer(handle_, &raw), "MV_CC_FreeImageBuffer");
  return image;
}

void Camera::close() noexcept
{
  if (handle_ == nullptr) return;
  if (grabbing_) MV_CC_StopGrabbing(handle_);
  if (opened_) MV_CC_CloseDevice(handle_);
  MV_CC_DestroyHandle(handle_);
  handle_ = nullptr;
  opened_ = false;
  grabbing_ = false;
}

Camera::~Camera()
{
  close();
}

}  // namespace io
