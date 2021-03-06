cmake_minimum_required(VERSION 3.10)

add_library(CONV_LAYER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/conv_layer.cpp)
target_include_directories(CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(CONV_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_include_directories(CONV_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})
target_compile_options(CONV_LAYER PRIVATE -fno-builtin)

add_library(DENSE_LAYER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/dense_layer.cpp)
target_include_directories(DENSE_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(DENSE_LAYER PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_include_directories(DENSE_LAYER PUBLIC ${VIVADO_INCLUDE_DIRS})

add_library(MEMORY_MANAGER STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/memory_manager.cpp)
target_include_directories(MEMORY_MANAGER PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(MEMORY_MANAGER PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_link_libraries(MEMORY_MANAGER CONV_LAYER)
target_link_libraries(MEMORY_MANAGER DENSE_LAYER)

add_library(DQNET_UTILS STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/dqnet_utils.cpp)
target_include_directories(DQNET_UTILS PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(DQNET_UTILS PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_link_libraries(DQNET_UTILS CONV_LAYER)
target_link_libraries(DQNET_UTILS DENSE_LAYER)

add_library(DQNET STATIC ${CMAKE_SOURCE_DIR}/src/dqnet/dqnet.cpp)
target_include_directories(DQNET PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(DQNET PUBLIC ${CMAKE_SOURCE_DIR}/include/dqnet)
target_link_libraries(DQNET CONV_LAYER)
target_link_libraries(DQNET DENSE_LAYER)
