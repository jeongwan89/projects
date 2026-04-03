cmake_minimum_required(VERSION 3.13)

# Pico SDK 경로 설정 (환경 변수 또는 직접 지정)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(blink_led C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(blink_led main.cpp)

# 표준 라이브러리 링크
target_link_libraries(blink_led pico_stdlib)

# USB 시리얼 활성화 (stdio)
pico_enable_stdio_usb(blink_led 1)
pico_enable_stdio_uart(blink_led 0)

# UF2/BIN/HEX 파일 생성
pico_add_extra_outputs(blink_led)
