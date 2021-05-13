cmake_minimum_required(VERSION 3.10)

add_library(CONV_LAYER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/conv_layer.cpp)
target_include_directories(CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(CONV_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})