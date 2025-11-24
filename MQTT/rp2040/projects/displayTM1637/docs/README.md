# displayTM1637 - MQTT 센서 데이터 디스플레이

## 프로젝트 개요
MQTT 브로커에서 수신한 센서 데이터(온도/습도)를 TM1637 4-digit 7-segment LED 디스플레이에 실시간으로 표시하는 시스템입니다.

## 주요 기능
- ✅ WiFi 연결 (ESP-01 모듈)
- ✅ MQTT 브로커 연결 및 자동 재연결
- ✅ 다중 센서 토픽 구독 (GH1~GH4 온실)
- ✅ TM1637 디스플레이 제어
- ✅ 자동 모드 회전 (5초마다 데이터 전환)
- ✅ 수동 모드 제어 (MQTT 명령)

## 하드웨어 연결

### RP2040 핀 배치
```
RP2040 Pico
├─ GPIO 3  → ESP-01 RST
├─ GPIO 4  → ESP-01 TX (UART1 TX)
├─ GPIO 5  → ESP-01 RX (UART1 RX)
├─ GPIO 10 → TM1637 CLK
├─ GPIO 11 → TM1637 DIO
└─ 3.3V, GND → 각 모듈 전원
```

### ESP-01 WiFi 모듈
```
ESP-01
├─ VCC  → 3.3V
├─ GND  → GND
├─ TX   → RP2040 GPIO 5 (RX)
├─ RX   → RP2040 GPIO 4 (TX)
└─ RST  → RP2040 GPIO 3
```

### TM1637 디스플레이
```
TM1637
├─ VCC  → 5V (또는 3.3V)
├─ GND  → GND
├─ CLK  → RP2040 GPIO 10
└─ DIO  → RP2040 GPIO 11
```

## MQTT 토픽 구조

### 구독 토픽 (Subscription)
```
Sensor/GH1/Center/Temp  - 온실1 온도
Sensor/GH1/Center/Hum   - 온실1 습도
Sensor/GH2/Center/Temp  - 온실2 온도
Sensor/GH2/Center/Hum   - 온실2 습도
Sensor/GH3/Center/Temp  - 온실3 온도
Sensor/GH3/Center/Hum   - 온실3 습도
Sensor/GH4/Center/Temp  - 온실4 온도
Sensor/GH4/Center/Hum   - 온실4 습도
Display/TM1637/mode     - 디스플레이 모드 제어
```

### 발행 토픽 (Publish)
```
Display/TM1637/status   - 디바이스 상태 (online/offline)
```

## 디스플레이 모드

### 자동 회전 모드 (기본값)
5초마다 자동으로 다음 모드로 전환됩니다:
```
GH1 온도 → GH1 습도 → GH2 온도 → GH2 습도 → ...
```

### 수동 모드
MQTT 토픽 `Display/TM1637/mode`에 숫자를 발행하여 고정 모드 설정:
```
0 - GH1 온도
1 - GH1 습도
2 - GH2 온도
3 - GH2 습도
4 - GH3 온도
5 - GH3 습도
6 - GH4 온도
7 - GH4 습도
8 - 자동 회전 모드
```

**예시:**
```bash
# MQTT 메시지 발행 (mosquitto_pub 사용)
mosquitto_pub -h 192.168.0.24 -u farmmain -P eerrtt -t "Display/TM1637/mode" -m "0"
```

## 디스플레이 표시 형식

### 온도 표시
```
25.3°C → "25.3" (소수점 1자리)
```

### 습도 표시
```
65.5% → "65.5" (소수점 1자리)
```

### 데이터 없음
```
센서 값이 없을 때 → "----" (빈칸)
```

## 빌드 및 실행

### 빌드
```bash
cd projects/displayTM1637
./build.sh
```

### RP2040 업로드
1. RP2040 BOOTSEL 버튼을 누른 상태로 USB 연결
2. RPI-RP2 드라이브가 마운트되면:
```bash
cp build/rp2040_display_tm1637.uf2 /media/$USER/RPI-RP2/
```

### 시리얼 모니터
```bash
# 시리얼 출력 확인
screen /dev/ttyACM0 115200
# 또는
minicom -D /dev/ttyACM0 -b 115200
```

## 설정 변경

### WiFi/MQTT 설정
`inc/config.h` 파일 수정:
```cpp
#define WIFI_SSID     "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define MQTT_BROKER   "your_broker_ip"
#define MQTT_USERNAME "your_username"
#define MQTT_PASSWORD "your_password"
```

### TM1637 핀 변경
```cpp
#define TM1637_CLK_PIN  10  // 클럭 핀
#define TM1637_DIO_PIN  11  // 데이터 핀
#define TM1637_BRIGHTNESS 5 // 밝기 (0-7)
```

## 문제 해결

### 디스플레이가 켜지지 않음
- TM1637 전원 연결 확인 (VCC, GND)
- CLK, DIO 핀 연결 확인
- 밝기 설정 확인 (TM1637_BRIGHTNESS)

### WiFi 연결 실패
- ESP-01 전원 확인 (3.3V)
- UART 핀 연결 확인 (TX↔RX 교차)
- WiFi SSID/비밀번호 확인

### MQTT 연결 실패
- 브로커 IP 주소 확인
- 사용자명/비밀번호 확인
- 네트워크 연결 확인

### 센서 데이터가 표시되지 않음
- MQTT 브로커에서 센서 데이터 발행 확인
- 토픽 이름 일치 확인
- 시리얼 모니터에서 `[수신]` 로그 확인

## 개발 정보

**개발 환경:**
- Pico SDK 1.5+
- CMake 3.13+
- GCC ARM 10.3+

**의존성:**
- `components/wifi_mqtt` - WiFi/MQTT 통신
- `components/actuators/tm1637` - TM1637 디스플레이 제어

**라이선스:** MIT
