add_library(io STATIC camera.cpp)

target_include_directories(io PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/hikrobot/include
  ${OpenCV_INCLUDE_DIRS}
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
  set(HIKROBOT_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hikrobot/lib/amd64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
  set(HIKROBOT_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hikrobot/lib/arm64")
else()
  message(FATAL_ERROR "Unsupported camera SDK architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

target_link_directories(io PUBLIC ${HIKROBOT_LIBRARY_DIR})
target_link_libraries(io PUBLIC ${OpenCV_LIBS} MvCameraControl usb-1.0 yaml-cpp)
