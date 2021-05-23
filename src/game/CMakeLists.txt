cmake_minimum_required(VERSION 3.10)

# NOTE: Each library/object will have an identifier and that identifier will then
# be used to link the final executable, i.e. target_link_libraries(ProjectName LibraryName).
# NOTE: The includes in the header files will be flat, so that they can be easily imported in Vivado.
add_library(BALL STATIC ${CMAKE_SOURCE_DIR}/src/game/ball.cpp)
target_include_directories(BALL PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(BALL PUBLIC ${CMAKE_SOURCE_DIR}/include/game)
target_include_directories(BALL PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(GAME_UTILS STATIC ${CMAKE_SOURCE_DIR}/src/game/game_utils.cpp)
target_include_directories(GAME_UTILS PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(GAME_UTILS PUBLIC ${CMAKE_SOURCE_DIR}/include/game)
target_include_directories(GAME_UTILS PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(GAME STATIC ${CMAKE_SOURCE_DIR}/src/game/game.cpp)
target_include_directories(GAME PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(GAME PUBLIC ${CMAKE_SOURCE_DIR}/include/game)
target_include_directories(GAME PUBLIC ${VIVADO_INCLUDE_DIRS})
target_link_libraries(GAME BALL)
target_link_libraries(GAME GAME_UTILS)