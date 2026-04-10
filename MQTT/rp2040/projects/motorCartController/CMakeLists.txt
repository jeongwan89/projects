cmake_minimum_required(VERSION 3.12)

# Pico SDK 초기화
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(motorCartController C CXX ASM)

# Pico SDK 초기 설정
pico_sdk_init()

# 프로젝트 이름 및 버전
set(PROJECT_NAME motorCartController)

# Include 디렉토리 설정
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 메인 실행 파일 생성
add_executable(${PROJECT_NAME}
    src/main.c
    src/motor_driver.c
    src/esp01_adapter.cpp
    src/mqtt_adapter.cpp
)

# Pico SDK 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    hardware_uart
    hardware_gpio
    hardware_pwm
)

# UART 활성화 (디버그용 USB UART)
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# wifi_mqtt 라이브러리 추가 (rp2040/components/wifi_mqtt)
set(WIFI_MQTT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../components/wifi_mqtt)
if(EXISTS ${WIFI_MQTT_PATH}/CMakeLists.txt)
    add_subdirectory(${WIFI_MQTT_PATH}
                     ${CMAKE_CURRENT_BINARY_DIR}/wifi_mqtt)
    target_link_libraries(${PROJECT_NAME}
        wifi_mqtt
    )
else()
    message(WARNING "wifi_mqtt library not found at ${WIFI_MQTT_PATH}")
endif()

# UF2 파일 생성 (Pico 부팅로더용)
pico_add_extra_outputs(${PROJECT_NAME})
