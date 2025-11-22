# RP2040 MQTT 프로젝트

## 프로젝트 개요

Raspberry Pi RP2040 마이크로컨트롤러와 ESP-01 WiFi 모듈을 사용하여 MQTT 통신을 구현하는 임베디드 시스템 프로젝트입니다. 모든 기능은 재사용 가능한 컴포넌트로 모듈화되어 있으며, 보안 강화와 안정성에 중점을 두고 개발되었습니다.

### 현재 개발 상태 (2025년 11월 23일)

- ✅ **핵심 통신 스택 완료**: ESP-01 WiFi + MQTT 클라이언트 (C++ 구현)
- ✅ **보안 강화 완료**: 전체 컴포넌트 취약점 패치 (Priority 1&2)
- ✅ **connectBroker 프로젝트**: MQTT 기본 통신 및 메시지 처리 구현
- 🚧 **센서/액츄에이터**: 구조 준비 완료, 실제 구현 대기

## 빠른 시작

### 1. 필수 요구사항

**하드웨어**:
- Raspberry Pi Pico (RP2040)
- ESP-01 WiFi 모듈
- USB 케이블 (Pico 연결)
- 점퍼 와이어

**소프트웨어**:
- Pico SDK (환경변수 `PICO_SDK_PATH` 설정)
- CMake (≥3.13)
- gcc-arm-none-eabi
- build-essential

### 2. 하드웨어 연결

```
RP2040 Pico         ESP-01
-----------         ------
GPIO 4 (TX1)  -->   RX
GPIO 5 (RX1)  <--   TX
GPIO 3        -->   RST
GND           -->   GND
3.3V          -->   VCC/CH_PD
```

### 3. 빌드 및 업로드

```bash
cd projects/connectBroker
./build.sh

# BOOTSEL 모드로 Pico 연결 후
cp build/rp2040_mqtt_client.uf2 /media/$USER/RPI-RP2/
```

### 4. 설정 파일 수정

`projects/connectBroker/inc/config.h`에서 WiFi 및 MQTT 설정:

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define MQTT_BROKER "broker.emqx.io"
#define MQTT_PORT 1883
```

## 프로젝트 구조

```
rp2040/
├── components/              # 재사용 가능한 컴포넌트
│   ├── wifi_mqtt/          # WiFi & MQTT 통신 (완료, 보안 강화)
│   │   ├── inc/            # uart_comm.h, esp01.h, mqtt_client.h
│   │   ├── src/            # C++ 구현 (.cpp 파일)
│   │   └── CMakeLists.txt
│   ├── sensors/            # 센서 드라이버 (구조 준비)
│   │   ├── dht22/
│   │   ├── bmp280/
│   │   ├── ds18b20/
│   │   └── adc_sensor/
│   └── actuators/          # 액츄에이터 (구조 준비)
│       ├── relay/
│       ├── servo/
│       ├── pwm_led/
│       └── motor/
└── projects/               # 실제 애플리케이션
    ├── connectBroker/      # MQTT 기본 통신 (완료)
    ├── temp_monitor/       # 온습도 모니터링 (계획)
    └── smart_farm/         # 스마트팜 제어 (계획)
```

## 주요 기능

### connectBroker 프로젝트 (현재 구현)

- ESP-01 WiFi 연결 및 관리
- MQTT 브로커 연결/해제
- 토픽 구독 및 메시지 발행
- 메시지 수신 및 처리
- LWT (Last Will Testament) 지원
- 시리얼 브리지 모드 (디버깅)

### 보안 강화 기능

- ✅ Buffer overflow 방지 (모든 AT 명령어)
- ✅ NULL pointer 검증 (모든 함수)
- ✅ Integer overflow 방지 (데이터 파싱)
- ✅ Race condition 해결 (UART 링버퍼 Critical Section)
- ✅ Format string 공격 방지
- ✅ GPIO/Port 범위 검증

## 모듈 사용 방법

### WiFi/MQTT 컴포넌트

```cmake
# CMakeLists.txt
add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt wifi_mqtt)
target_link_libraries(${PROJECT_NAME} wifi_mqtt)
```

```cpp
// main.cpp
#include "esp01.h"
#include "mqtt_client.h"

// Struct 기반 초기화 (call-by-reference)
Esp01Module esp01 = { /* 설정 */ };
MqttClient mqtt = { /* 설정 */ };

esp01_module_init(esp01);
mqtt_connect(mqtt);
mqtt_publish(mqtt, "topic", "message", 0, 0);
```

### 센서/액츄에이터 (계획)

```cmake
# 필요한 컴포넌트만 선택
add_subdirectory(../../components/sensors/dht22 dht22)
add_subdirectory(../../components/actuators/relay relay)
target_link_libraries(${PROJECT_NAME} dht22 relay)
```

## 개발 로드맵

### 완료 ✅
- ESP-01 WiFi 모듈 제어
- MQTT 클라이언트 구현
- 보안 취약점 패치 (23회 연속 빌드 성공)
- Struct 기반 아키텍처 리팩토링

### 진행 중 🚧
- 센서 컴포넌트 구현 (DHT22 우선)
- 액츄에이터 컴포넌트 구현 (Relay 우선)

### 계획 📋
- temp_monitor 프로젝트 (온습도 모니터링)
- smart_farm 프로젝트 (센서+액츄에이터 통합)
- OTA 펌웨어 업데이트
- 전력 관리 (Sleep 모드)

## 장점

1. **모듈화**: 각 기능이 독립적인 컴포넌트로 분리
2. **재사용성**: 센서/액츄에이터를 레고처럼 조합
3. **보안**: 체계적인 취약점 검증 및 패치
4. **유지보수**: 버그 수정이 모든 프로젝트에 자동 반영
5. **확장성**: 새 컴포넌트 추가 용이
6. **타입 안전성**: C++ Struct 기반 call-by-reference

## 문제 해결

### 빌드 오류
- `PICO_SDK_PATH` 환경변수 확인
- Pico SDK 버전 확인 (≥1.5.0 권장)

### WiFi 연결 실패
- ESP-01 전원 공급 확인 (3.3V, 최소 500mA)
- UART 연결 확인 (TX↔RX 교차 연결)
- SSID/비밀번호 확인

### MQTT 연결 실패
- 브로커 주소 및 포트 확인
- 네트워크 방화벽 설정 확인
- USB 시리얼 로그 확인 (`screen /dev/ttyACM0 115200`)

## 참고 문서

- **ARCHITECTURE.md**: 상세 아키텍처 및 컴포넌트 설명
- **projects/connectBroker/README.md**: connectBroker 프로젝트 사용법
- **components/wifi_mqtt/README.md**: WiFi/MQTT 컴포넌트 API 문서

## 라이선스

MIT License

## 기여

이슈 및 풀 리퀘스트 환영합니다!
