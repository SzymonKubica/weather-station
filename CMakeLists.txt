# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS
  main/libs/ssd1306
  main/libs/infrared-receiver
  main/libs/wifi
  )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(weather-station)

