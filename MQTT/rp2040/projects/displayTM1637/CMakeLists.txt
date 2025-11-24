cmake_minimum_required(VERSION 3.13)

# Pico SDK 경로 설정
include(pico_sdk_import.cmake)

# 프로젝트 정의
project(rp2040_display_tm1637 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pico SDK 초기화
pico_sdk_init()

# WiFi/MQTT 컴포넌트 추가
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt wifi_mqtt)

# TM1637 컴포넌트 추가
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/actuators/tm1637 tm1637)

# 실행 파일 생성
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# 헤더 파일 경로
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_uart
    hardware_gpio
    wifi_mqtt
    tm1637
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
