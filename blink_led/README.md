# Blink LED 프로젝트

이 프로젝트는 다양한 MCU에서 LED를 깜빡이는 기본 예제입니다.

## 프로젝트 구조

```
blink_led/
├── stm32/          # STM32 마이크로컨트롤러용 코드
├── arduino/        # Arduino 보드용 코드
├── esp32/          # ESP32 마이크로컨트롤러용 코드
└── rp2040/         # Raspberry Pi RP2040 (Pico)용 코드
```

## 지원 MCU

### STM32
- STM32F4 시리즈
- HAL 라이브러리 사용

### Arduino
- Arduino Uno
- Arduino Mega
- Arduino Nano

### ESP32
- ESP32-WROOM-32
- ESP-IDF 또는 Arduino 프레임워크 사용

### RP2040
- Raspberry Pi Pico / Pico W
- Pico SDK (CMake) 사용
- ⚠️ 여러 번 플래시 후 마운트 안 될 때 → [README 참조](rp2040/README.md#️-rp2040이-마운트되지-않을-때-bootsel-모드-진입-방법)

## 빌드 및 실행

각 MCU 폴더 내의 README.md를 참조하세요.
