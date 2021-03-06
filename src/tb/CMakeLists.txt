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

add_executable(TEST_DENSE_LAYER ${CMAKE_SOURCE_DIR}/src/tb/test_dense_layer.cpp)
target_include_directories(TEST_DENSE_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_DENSE_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})
target_link_libraries(TEST_DENSE_LAYER CONV_LAYER)

add_executable(TEST_DQNET ${CMAKE_SOURCE_DIR}/src/tb/test_dqnet.cpp)
target_include_directories(TEST_DQNET PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_DQNET PUBLIC ${VIVADO_INCLUDE_DIRS})
target_link_libraries(TEST_DQNET DQNET)

add_executable(TEST_GAME ${CMAKE_SOURCE_DIR}/src/tb/test_game.cpp)
target_include_directories(TEST_GAME PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_GAME PUBLIC ${VIVADO_INCLUDE_DIRS})
target_include_directories(TEST_GAME PUBLIC ${OpenCv_INCLUDE_DIRS})
target_link_libraries(TEST_GAME ${OpenCv_LIBS})
target_link_libraries(TEST_GAME GAME)

add_executable(TEST_PONG ${CMAKE_SOURCE_DIR}/src/tb/test_pong.cpp)
target_include_directories(TEST_PONG PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(TEST_PONG PUBLIC ${VIVADO_INCLUDE_DIRS})
target_include_directories(TEST_PONG PUBLIC ${OpenCv_INCLUDE_DIRS})
target_link_libraries(TEST_PONG ${OpenCv_LIBS})
target_link_libraries(TEST_PONG PONG)

# add_test(NAME TestOpenCv COMMAND OPENCV_TEST)
# add_test(NAME TestAxisLib COMMAND TEST_AXIS_LIB)
# add_test(NAME TestConvLayer COMMAND TEST_CONV_LAYER)
add_test(NAME TestDenseLayer COMMAND TEST_DENSE_LAYER)
# add_test(NAME TestDQNet COMMAND TEST_DQNET)
# add_test(NAME TestGame COMMAND TEST_GAME)
# add_test(NAME TestPong COMMAND TEST_PONG)