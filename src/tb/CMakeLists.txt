cmake_minimum_required(VERSION 3.10)

add_executable(OPENCV_TEST ${CMAKE_SOURCE_DIR}/src/tb/test_hdmi.cpp)
target_include_directories(OPENCV_TEST PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(OPENCV_TEST PUBLIC ${VIVADO_INCLUDE_DIRS})
target_include_directories(OPENCV_TEST PUBLIC ${OpenCv_INCLUDE_DIRS})
target_link_libraries(OPENCV_TEST ${OpenCv_LIBS})

add_executable(TEST_AXIS_LIB ${CMAKE_SOURCE_DIR}/src/tb/test_axis_lib.cpp)
target_include_directories(TEST_AXIS_LIB PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_AXIS_LIB PUBLIC ${VIVADO_INCLUDE_DIRS})
target_include_directories(TEST_AXIS_LIB PUBLIC ${OpenCv_INCLUDE_DIRS})
target_link_libraries(TEST_AXIS_LIB ${OpenCv_LIBS})
target_link_libraries(TEST_AXIS_LIB AXIS_LIB)

add_executable(TEST_CONV_LAYER ${CMAKE_SOURCE_DIR}/src/tb/test_conv_layer.cpp)
target_include_directories(TEST_CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_CONV_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})
target_link_libraries(TEST_CONV_LAYER CONV_LAYER)

add_test(NAME TestOpenCv COMMAND OPENCV_TEST)
add_test(NAME TestAxisLib COMMAND TEST_AXIS_LIB)
add_test(NAME TestConvLayer COMMAND TEST_CONV_LAYER)