# ESP32 Blink LED

ESP32 마이크로컨트롤러에서 LED를 깜빡이는 프로젝트입니다.

## 하드웨어 요구사항

- ESP32 개발 보드 (ESP32-WROOM-32)
- USB 케이블 (데이터 전송 지원)

## 개발 환경

### 방법 1: ESP-IDF 사용
- ESP-IDF v5.0 이상
- Python 3.7 이상

### 방법 2: Arduino IDE 사용
- Arduino IDE
- ESP32 보드 패키지

## 빌드 및 플래시 방법

### ESP-IDF 사용 시

```bash
# 프로젝트 설정
idf.py set-target esp32

# 빌드
idf.py build

# 플래시 및 모니터
idf.py -p /dev/ttyUSB0 flash monitor
```

### Arduino IDE 사용 시

1. Arduino IDE에서 `blink_led.ino` 파일 열기
2. 보드 선택: Tools > Board > ESP32 Dev Module
3. 포트 선택: Tools > Port > (해당 포트)
4. 업로드

## 핀 설정

- LED 핀: GPIO2 (내장 LED)
