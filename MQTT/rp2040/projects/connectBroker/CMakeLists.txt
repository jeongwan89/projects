
cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD pico CACHE STRING "Board type")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pico SDK import
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)
elseif(DEFINED ENV{PICO_SDK_PATH})
    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
else()
    message(FATAL_ERROR "Pico SDK를 찾을 수 없습니다. 환경변수 PICO_SDK_PATH를 설정하거나 프로젝트 루트에 pico_sdk_import.cmake를 두세요.")
endif()

project(rp2040_mqtt_client C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

# include 디렉토리
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# wifi_mqtt 모듈 추가 (빌드 아웃풋 분리)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt ${CMAKE_BINARY_DIR}/wifi_mqtt)

# 실행 파일(main.c만)
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_uart
    hardware_gpio
    wifi_mqtt
)

# USB/UF2 설정
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})
