project(EasySound)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(easy_sound src/easy_sound.cpp include/easy_sound.h)
target_link_libraries(easy_sound ${CONAN_TARGETS})
