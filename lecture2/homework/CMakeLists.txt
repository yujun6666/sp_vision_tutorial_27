cmake_minimum_required(VERSION 3.16.3)

project(lecture2_homework LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()



# =========================
# OpenCV
# =========================

find_package(OpenCV REQUIRED)


# =========================
# other dependencies
# =========================

find_package(fmt REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)



# =========================
# OpenVINO
# =========================

set(OpenVINO_DIR
    "/opt/intel/openvino_2024.6.0/runtime/cmake"
    CACHE PATH
    "Directory containing OpenVINOConfig.cmake"
)

find_package(OpenVINO REQUIRED COMPONENTS Runtime)



# =========================
# include
# =========================

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${OpenCV_INCLUDE_DIRS})



# =========================
# libraries
# =========================

add_subdirectory(io)
add_subdirectory(tools)
add_subdirectory(tasks)



# =========================
# camera test
# =========================

add_executable(camera_test
    camera_test.cpp
)

target_link_libraries(camera_test
    PRIVATE
    io
)



# =========================
# YOLO armor detection
# =========================

add_executable(main
    main.cpp
)

target_link_libraries(main
    PRIVATE
    io
    tools
    tasks
)



# =========================
# AprilTag extra task
# =========================

add_executable(opencv
    opencv.cpp
)

target_link_libraries(opencv
    PRIVATE
    io
    tools
    tasks
    ${OpenCV_LIBS}
)



# =========================
# example
# =========================

add_executable(example EXCLUDE_FROM_ALL
    io/example.cpp
)

target_link_libraries(example
    PRIVATE
    io
)