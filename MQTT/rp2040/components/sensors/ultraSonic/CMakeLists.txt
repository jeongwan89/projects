# HC-SR04 ultrasonic sensor driver

cmake_minimum_required(VERSION 3.13)

add_library(ultraSonic STATIC
    src/ultrasonic.c
)

target_include_directories(ultraSonic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(ultraSonic
    pico_stdlib
    hardware_gpio
)
