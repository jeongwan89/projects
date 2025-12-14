cmake_minimum_required(VERSION 3.13)

# Pico SDK 경로 설정
include(pico_sdk_import.cmake)

# 프로젝트 정의
project(tm1637_test C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# compile_commands.json 생성
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico SDK 초기화
pico_sdk_init()

# TM1637 라이브러리
add_library(tm1637_lib
    src/tm1637.cpp
)

target_include_directories(tm1637_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(tm1637_lib
    pico_stdlib
    hardware_gpio
)

# 테스트 실행 파일
add_executable(${PROJECT_NAME}
    main.cpp
)

# 헤더 파일 경로
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_gpio
    tm1637_lib
)

# USB 출력 활성화
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# UF2 파일 생성
pico_add_extra_outputs(${PROJECT_NAME})

# 컴파일 옵션
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
)
