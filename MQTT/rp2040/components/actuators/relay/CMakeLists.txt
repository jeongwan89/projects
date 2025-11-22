# 릴레이 제어 컴포넌트

cmake_minimum_required(VERSION 3.13)

add_library(relay STATIC
    src/relay.c
)

target_include_directories(relay PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(relay
    pico_stdlib
    hardware_gpio
)
