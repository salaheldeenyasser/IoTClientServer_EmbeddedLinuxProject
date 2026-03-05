cmake_minimum_required(VERSION 3.16)

project(iot-client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(iot-client main.cpp)

target_include_directories(iot-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS iot-client DESTINATION bin)
