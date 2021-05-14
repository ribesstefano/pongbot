cmake_minimum_required(VERSION 3.10)

add_library(CONV_LAYER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/conv_layer.cpp)
target_include_directories(CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_include_directories(CONV_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(DENSE_LAYER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/dense_layer.cpp)
target_include_directories(DENSE_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(DENSE_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_include_directories(DENSE_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(DQNET STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/dqnet.cpp)
target_include_directories(DQNET PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(DQNET PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_link_libraries(DQNET CONV_LAYER)
target_link_libraries(DQNET DENSE_LAYER)
