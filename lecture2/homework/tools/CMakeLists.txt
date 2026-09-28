add_library(tools OBJECT
  img_tools.cpp
  logger.cpp
)

target_link_libraries(tools PUBLIC ${OpenCV_LIBS} fmt::fmt spdlog::spdlog)
