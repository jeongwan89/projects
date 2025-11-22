# WiFi & MQTT 통신 컴포넌트
# ESP-01 기반 WiFi 모듈과 MQTT 통신 기능 제공

cmake_minimum_required(VERSION 3.13)

# 라이브러리 생성
add_library(wifi_mqtt STATIC
    src/uart_comm.cpp
    src/esp01.cpp
    src/mqtt_client.cpp
    src/serial_bridge.cpp
)

# 인클루드 디렉토리 설정
target_include_directories(wifi_mqtt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/projects/connectBroker/inc
)

# Pico SDK 라이브러리 링크
target_link_libraries(wifi_mqtt
    pico_stdlib
    pico_stdio_usb
    hardware_uart
    hardware_gpio
)

# 사용 예시:
# add_subdirectory(path/to/components/wifi_mqtt wifi_mqtt)
# target_link_libraries(your_project wifi_mqtt)
