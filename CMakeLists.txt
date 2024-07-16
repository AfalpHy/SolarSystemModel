cmake_minimum_required(VERSION 3.10)

project(SolarSystemModel)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} SDL2 SDL2_image SDL2_mixer)
