# RP2040 ESP-01 MQTT 프로젝트

## 개요
RP2040 마이크로컨트롤러와 ESP-01 WiFi 모듈을 사용하여 MQTT 브로커에 연결하는 프로젝트입니다.

## 프로젝트 구조

```
connectBroker/
├── docs/           # 문서 파일
│   └── README.md
├── inc/            # 헤더 파일
│   ├── config.h        # 시스템 설정 및 상수 정의
│   ├── uart_comm.h     # UART 통신 함수 선언
│   ├── esp01.h         # ESP-01 제어 함수 선언
│   └── mqtt_client.h   # MQTT 클라이언트 함수 선언
├── src/            # 소스 파일
│   ├── main.c          # 메인 프로그램
│   ├── uart_comm.c     # UART 통신 구현
│   ├── esp01.c         # ESP-01 제어 구현
│   └── mqtt_client.c   # MQTT 클라이언트 구현
└── lib/            # 외부 라이브러리 (필요시)
```

## 모듈 설명

### 1. config.h
- UART, WiFi, MQTT 설정 상수 정의
- 핀 배치, 통신 속도, 네트워크 설정 등

### 2. uart_comm 모듈
- ESP-01과의 UART 통신 담당
- AT 명령어 송수신 기능
- 응답 대기 및 버퍼 관리

### 3. esp01 모듈
- ESP-01 WiFi 모듈 초기화
- WiFi 연결 관리
- 네트워크 상태 확인

### 4. mqtt_client 모듈
- MQTT 브로커 연결
- 메시지 발행(Publish)
- 토픽 구독(Subscribe)
- 수신 메시지 처리

### 5. main.c
- 전체 프로그램 흐름 제어
- 주기적인 센서 데이터 전송
- Alive 메시지 전송
- 메시지 수신 처리

## 하드웨어 연결

```
RP2040          ESP-01
GPIO 4 (TX) --> RX
GPIO 5 (RX) <-- TX
3.3V        --> VCC, CH_PD
GND         --> GND
```

**중요**: ESP-01의 RST 핀을 RP2040의 GPIO 3에 연결하여 하드웨어 리셋을 제어합니다. 
이를 통해 RP2040이 완전히 부팅된 후 ESP-01을 안정적으로 초기화할 수 있습니다.

## 기능

1. **하드웨어 리셋 제어**: GPIO를 통한 ESP-01 리셋 제어로 안정적인 초기화
2. **WiFi 연결**: ESP-01을 통한 무선 네트워크 연결
3. **MQTT 통신**: 브로커와 양방향 통신
4. **센서 데이터 전송**: 10초마다 센서 데이터 발행
5. **Alive 신호**: 60초마다 생존 신호 전송
6. **제어 명령 수신**: 제어 토픽으로부터 명령 수신

## MQTT 토픽

- `test/rp2040/alive` - Alive 메시지
- `test/rp2040/sensor` - 센서 데이터
- `test/rp2040/control` - 제어 명령 (구독)

## 설정 변경

`inc/config.h` 파일에서 다음 설정을 변경할 수 있습니다:

- WiFi SSID 및 비밀번호
- MQTT 브로커 주소 및 포트
- MQTT 사용자 이름 및 비밀번호
- UART 핀 배치
- 데이터 전송 주기

## 빌드 및 실행

### 사전 요구사항

1. **Pico SDK 설치**
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   ```

2. **환경변수 설정**
   ```bash
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

3. **필요한 도구 설치**
   ```bash
   sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
   ```

### 빌드 방법

#### 방법 1: 빌드 스크립트 사용 (권장)
```bash
./build.sh
```

#### 방법 2: 수동 빌드
```bash
mkdir build
cd build
cmake ..
make -j4
```

### 빌드 결과

빌드가 성공하면 `build/` 디렉토리에 다음 파일들이 생성됩니다:
- `rp2040_mqtt_client.uf2` - RP2040 업로드용 파일
- `rp2040_mqtt_client.elf` - 디버깅용 ELF 파일
- `rp2040_mqtt_client.bin` - 바이너리 파일

### RP2040에 업로드

1. RP2040의 BOOTSEL 버튼을 누른 상태로 USB 연결
2. RPI-RP2 드라이브가 마운트되면 UF2 파일을 복사
   ```bash
   cp build/rp2040_mqtt_client.uf2 /media/$USER/RPI-RP2/
   ```
3. 자동으로 재부팅되며 프로그램 실행

## TODO

- [ ] 실제 센서 데이터 읽기 구현
- [ ] LED 제어 기능 구현
- [ ] 에러 처리 강화
- [ ] 재연결 로직 추가
