cmake_minimum_required(VERSION 3.31)
project(BigBalls C)

set(CMAKE_C_STANDARD 17)
add_executable(BigBalls game.c
        player.h
        collider.h
        food.h
        player.c
        food.c
)
target_link_libraries(BigBalls PRIVATE raylib)
find_package(raylib CONFIG REQUIRED)