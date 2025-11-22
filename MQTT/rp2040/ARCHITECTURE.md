# RP2040 MQTT 프로젝트 아키텍처

## 디렉토리 구조

```
/MQTT/rp2040/
├── components/                      # 재사용 가능한 컴포넌트들
│   ├── wifi_mqtt/                   # WiFi & MQTT 통신 컴포넌트
│   │   ├── inc/
│   │   │   ├── uart_comm.h
│   │   │   ├── esp01.h
│   │   │   ├── mqtt_client.h
│   │   │   ├── debug_log.h
│   │   │   └── serial_bridge.h
│   │   ├── src/
│   │   │   ├── uart_comm.c
│   │   │   ├── esp01.c
│   │   │   ├── mqtt_client.c
│   │   │   ├── debug_log.c
│   │   │   └── serial_bridge.c
│   │   └── CMakeLists.txt           # 정적 라이브러리
│   │
│   ├── sensors/                     # 센서 드라이버 컴포넌트
│   │   ├── dht22/                   # DHT22 온습도 센서
│   │   │   ├── inc/dht22.h
│   │   │   ├── src/dht22.c
│   │   │   └── CMakeLists.txt
│   │   ├── bmp280/                  # BMP280 기압/온도 센서
│   │   │   ├── inc/bmp280.h
│   │   │   ├── src/bmp280.c
│   │   │   └── CMakeLists.txt
│   │   ├── adc_sensor/              # ADC 기반 센서 (토양습도 등)
│   │   │   ├── inc/adc_sensor.h
│   │   │   ├── src/adc_sensor.c
│   │   │   └── CMakeLists.txt
│   │   └── ds18b20/                 # DS18B20 온도 센서
│   │       ├── inc/ds18b20.h
│   │       ├── src/ds18b20.c
│   │       └── CMakeLists.txt
│   │
│   └── actuators/                   # 액츄에이터 컴포넌트
│       ├── relay/                   # 릴레이 제어
│       │   ├── inc/relay.h
│       │   ├── src/relay.c
│       │   └── CMakeLists.txt
│       ├── servo/                   # 서보 모터
│       │   ├── inc/servo.h
│       │   ├── src/servo.c
│       │   └── CMakeLists.txt
│       ├── pwm_led/                 # PWM LED 제어
│       │   ├── inc/pwm_led.h
│       │   ├── src/pwm_led.c
│       │   └── CMakeLists.txt
│       └── motor/                   # DC 모터 제어
│           ├── inc/motor.h
│           ├── src/motor.c
│           └── CMakeLists.txt
│
├── projects/                        # 실제 애플리케이션 프로젝트들
│   ├── connectBroker/               # 기존 기본 연결 테스트
│   │   ├── src/main.c
│   │   ├── inc/config.h
│   │   └── CMakeLists.txt
│   │
│   ├── temp_monitor/                # 온습도 모니터링 프로젝트
│   │   ├── src/main.c
│   │   ├── inc/config.h
│   │   └── CMakeLists.txt
│   │
│   ├── smart_farm/                  # 스마트팜 제어 시스템
│   │   ├── src/
│   │   │   ├── main.c
│   │   │   ├── sensor_manager.c
│   │   │   └── actuator_manager.c
│   │   ├── inc/
│   │   │   ├── config.h
│   │   │   ├── sensor_manager.h
│   │   │   └── actuator_manager.h
│   │   └── CMakeLists.txt
│   │
│   └── env_controller/              # 환경 제어 시스템
│       ├── src/main.c
│       ├── inc/config.h
│       └── CMakeLists.txt
│
└── lib/                             # 외부 라이브러리 (옵션)
    └── README.md

```

## 컴포넌트 사용 방법

### 1. WiFi/MQTT 컴포넌트
모든 프로젝트의 기본 통신 레이어입니다.

```cmake
# 프로젝트 CMakeLists.txt에서
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt wifi_mqtt)
target_link_libraries(${PROJECT_NAME} wifi_mqtt)
```

### 2. 센서 컴포넌트
필요한 센서만 선택하여 사용합니다.

```cmake
# DHT22와 BMP280만 사용하는 경우
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/sensors/dht22 dht22)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/sensors/bmp280 bmp280)
target_link_libraries(${PROJECT_NAME} dht22 bmp280)
```

### 3. 액츄에이터 컴포넌트
필요한 액츄에이터만 선택하여 사용합니다.

```cmake
# 릴레이와 서보만 사용하는 경우
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/actuators/relay relay)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/actuators/servo servo)
target_link_libraries(${PROJECT_NAME} relay servo)
```

## 새 프로젝트 생성 예시

### temp_monitor 프로젝트 CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(temp_monitor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# 컴포넌트 추가
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt wifi_mqtt)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/sensors/dht22 dht22)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/sensors/bmp280 bmp280)

# 실행 파일
add_executable(${PROJECT_NAME}
    src/main.c
)

# 라이브러리 링크
target_link_libraries(${PROJECT_NAME}
    wifi_mqtt
    dht22
    bmp280
    pico_stdlib
    hardware_uart
    hardware_gpio
    hardware_i2c
)

# 설정 헤더 인클루드
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})
```

## 장점

1. **모듈화**: 각 기능이 독립적인 컴포넌트로 분리
2. **재사용성**: 센서/액츄에이터를 레고처럼 조합
3. **유지보수**: 버그 수정이 모든 프로젝트에 자동 반영
4. **확장성**: 새 센서/액츄에이터 추가가 용이
5. **테스트**: 각 컴포넌트를 독립적으로 테스트 가능

## 다음 단계

1. connectBroker 코드를 components/wifi_mqtt로 리팩토링
2. 센서 컴포넌트 작성 (DHT22부터 시작 권장)
3. 액츄에이터 컴포넌트 작성
4. 새 프로젝트 생성 및 컴포넌트 조합
