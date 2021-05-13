cmake_minimum_required(VERSION 3.10)

add_executable(HDMI_TEST ${CMAKE_SOURCE_DIR}/src/tb/test_hdmi.cpp)
target_include_directories(HDMI_TEST PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(HDMI_TEST PUBLIC ${VIVADO_INCLUDE_DIRS})
target_include_directories(HDMI_TEST PUBLIC ${OpenCv_INCLUDE_DIRS})
target_link_libraries(HDMI_TEST ${OpenCv_LIBS})

add_test(NAME PongBotTest COMMAND HDMI_TEST) #  WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})