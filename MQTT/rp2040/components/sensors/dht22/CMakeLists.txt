# DHT22 온습도 센서 드라이버

cmake_minimum_required(VERSION 3.13)

add_library(dht22 STATIC
    src/dht22.c
)

target_include_directories(dht22 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(dht22
    pico_stdlib
    hardware_gpio
)

