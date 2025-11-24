cmake_minimum_required(VERSION 3.13)

# TM1637 컴포넌트 라이브러리
add_library(tm1637
    src/tm1637.cpp
)

# 헤더 파일 경로
target_include_directories(tm1637 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Pico SDK 라이브러리 링크
target_link_libraries(tm1637
    pico_stdlib
    hardware_gpio
)
