# ESP07 HM100 MQTT Monitor - PlatformIO

ESP07 기반 HM-100 센서 모니터링 및 MQTT 전송 프로젝트

## 프로젝트 구조

```
ESP07_Read_HM100_MQTT_PlatformIO/
├── platformio.ini                          # PlatformIO 설정 파일
├── src/                                     # 소스 코드 디렉토리
│   ├── main.cpp                            # 메인 프로그램
│   └── ESP07_Read_HM100_MQTT_PlatformIO_src.cpp  # HM100 통신 로직
├── include/                                 # 헤더 파일 디렉토리
│   ├── ESP07_Read_HM100_MQTT_PlatformIO_src.h    # 함수 선언
│   ├── credentials.h                       # WiFi/MQTT 자격증명 (gitignore됨)
│   └── credentials.h.example               # 자격증명 템플릿
├── lib/                                     # 로컬 라이브러리 (선택사항)
├── .gitignore                              # Git 제외 설정
└── README.md                               # 프로젝트 문서

```

## 설치 및 설정

### 1. 필수 요구사항
- [PlatformIO IDE](https://platformio.org/install) 또는 PlatformIO Core
- ESP8266 보드 (ESP-07)
- HM-100 센서
- RS485 통신 모듈

### 2. 프로젝트 설정

1. 프로젝트 클론 또는 다운로드

2. 자격증명 파일 설정:
   ```bash
   cd include
   cp credentials.h.example credentials.h
   # credentials.h 파일을 편집하여 실제 정보 입력
   ```

3. `include/credentials.h` 파일 수정:
   ```cpp
   #define WIFI_SSID "실제_WiFi_SSID"
   #define WIFI_PASSWORD "실제_WiFi_비밀번호"
   #define MQTT_BROKER_IP "실제_MQTT_서버_IP"
   #define MQTT_USERNAME "실제_사용자명"
   #define MQTT_PASSWORD "실제_비밀번호"
   ```

4. RaiseEventClass 라이브러리 확인:
   - `platformio.ini`의 `lib_extra_dirs`에 라이브러리 경로가 설정되어 있습니다
   - 기본 경로: `/home/kjw/Git/Green_House/mylibraries`
   - 필요시 해당 경로를 수정하세요

### 3. 빌드 및 업로드

```bash
# PlatformIO Core 사용 시
pio run                  # 빌드
pio run --target upload  # 업로드
pio device monitor       # 시리얼 모니터

# VS Code PlatformIO 확장 사용 시
# 하단 도구바의 빌드/업로드 버튼 클릭
```

## 하드웨어 설정

### 핀 배치
- **RS485 RX**: GPIO 13
- **RS485 TX**: GPIO 12
- **RS485 TX Control**: GPIO 16
- **LED**: GPIO 2 (내장 LED)

### HM-100 센서
- Modbus RTU 프로토콜 사용
- 통신 속도: 19200 baud
- 측정값: EC, pH, 수온(Temp_drain)

## 기능

- ✅ HM-100 센서에서 RS485로 데이터 읽기
- ✅ EC, pH, 수온 측정
- ✅ MQTT 브로커로 데이터 전송
- ✅ WiFi 자동 연결 및 재연결
- ✅ OTA (Over The Air) 펌웨어 업데이트
- ✅ 웹 기반 펌웨어 업데이트
- ✅ Modbus CRC16 데이터 검증
- ✅ 버퍼 오버플로우 방지

## MQTT 토픽

프로젝트는 `platformio.ini`의 빌드 환경에 따라 다른 MQTT 토픽을 사용합니다:

### 예시 (Esp07_HM100_MQTT_04 설정 시):
- `Sensor/GH4/Rear/EC` - EC 값
- `Sensor/GH4/Rear/PH` - pH 값
- `Sensor/GH4/Rear/Temp_Drain` - 수온
- `Sensor/GH4/Rear/Stat` - 센서 상태

## 설정 변경

다른 위치/온실에 배포하려면 `include/ESP07_Read_HM100_MQTT_PlatformIO_src.h` 파일에서 정의 변경:

```cpp
// 현재 활성: Esp07_HM100_MQTT_04
// 다른 설정으로 변경하려면 주석 처리 후 원하는 설정 활성화
#define Esp07_HM100_MQTT_01  // 온실 1동
// #define Esp07_HM100_MQTT_02  // 온실 2동
// #define Esp07_HM100_MQTT_03  // 온실 3동
// #define Esp07_HM100_MQTT_04  // 온실 4동
// #define Esp07_HM100_MQTT_05  // 육묘장 1동
```

## 디버깅

시리얼 모니터 출력 (115200 baud):
- RS485 요청/응답 데이터 (16진수)
- 파싱된 센서 값
- MQTT 연결 상태
- CRC 검증 결과
- 에러 메시지

## 보안 고려사항

⚠️ **중요**: `credentials.h` 파일은 `.gitignore`에 포함되어 있어 Git에 추적되지 않습니다.
- 실제 자격증명을 코드 저장소에 커밋하지 마세요
- `credentials.h.example`을 템플릿으로 사용하세요
- 프로덕션 환경에서는 MQTT TLS/SSL 연결을 권장합니다

## 라이선스

프로젝트 라이선스 정보를 여기에 추가하세요.

## 문제 해결

### 빌드 오류
- RaiseEventClass 라이브러리 경로 확인
- `platformio.ini`의 `lib_extra_dirs` 경로가 올바른지 확인

### 연결 문제
- WiFi SSID/비밀번호 확인
- MQTT 브로커 IP 및 포트 확인
- 시리얼 모니터로 연결 상태 확인

### RS485 통신 문제
- 배선 확인 (A-A, B-B)
- HM-100 통신 설정 확인 (19200 baud, Modbus RTU)
- CRC 오류 메시지 확인
