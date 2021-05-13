cmake_minimum_required(VERSION 3.10)

# NOTE: Each library/object will have an identifier and that identifier will then
# be used to link the final executable, i.e. target_link_libraries(ProjectName LibraryName)
add_library(GAME STATIC ${CMAKE_SOURCE_DIR}/src/game/game.cpp)
target_include_directories(GAME PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(GAME PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(BALL STATIC ${CMAKE_SOURCE_DIR}/src/game/ball.cpp)
target_include_directories(BALL PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(BALL PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(POINT STATIC ${CMAKE_SOURCE_DIR}/src/game/point.cpp)
target_include_directories(POINT PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(POINT PUBLIC ${VIVADO_INCLUDE_DIRS})