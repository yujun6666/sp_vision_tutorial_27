add_library(tasks OBJECT
  armor.cpp
  yolo.cpp
  yolos/yolov5.cpp
)

if(BUILD_APRILTAG)
  target_sources(tasks PRIVATE apriltag_detector.cpp)
endif()

target_link_libraries(tasks PUBLIC
  io
  tools
  openvino::runtime
  Eigen3::Eigen
  fmt::fmt
  yaml-cpp
  ${OpenCV_LIBS}
)
