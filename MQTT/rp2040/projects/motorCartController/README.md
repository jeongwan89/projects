# motorCartController

RP2040 Pico 기반 4채널 모터 카트 컨트롤러 프로젝트입니다.

이 프로젝트는 다음 흐름으로 동작합니다.
1. ESP01(UART1) 초기화 및 Wi-Fi 연결
2. MQTT 브로커 연결
3. 모터 제어 토픽 구독
4. 수신 명령에 따라 Motor1~Motor4 제어
5. 모터 상태/비상 상태/디바이스 상태 발행

---

## 1) 프로젝트 구성

- `src/main.c`
  - 시스템 부팅 시퀀스
  - 비상 스위치 인터럽트 처리
  - MQTT 명령 수신 후 모터 제어
  - 상태 토픽 발행
- `src/motor_driver.c`
  - DRV8871 4유닛 제어
  - 정회전/역회전/정지/브레이크
  - 램프 제어, 동기 제어, 비상정지
- `src/esp01_adapter.cpp`
  - `Esp01Module` 구조체 기반 ESP01 초기화/연결 어댑터
- `src/mqtt_adapter.cpp`
  - `MqttClient` 구조체 기반 MQTT 연결/구독/발행 어댑터
- `include/motor_driver.h`
  - 모터 핀맵, API 선언
- `include/esp01_adapter.h`
  - ESP01 초기화/연결 C 인터페이스
- `include/mqtt_adapter.h`
  - MQTT 연결/구독/발행 C 인터페이스

---

## 2) 하드웨어 핀맵

### UART / ESP01
- UART1 TX: GPIO4
- UART1 RX: GPIO5
- ESP01 RST: GPIO18 (active-low)

### DRV8871 (4 Units)
- Motor1: PWM GPIO6, IN1 GPIO7, IN2 GPIO12
- Motor2: PWM GPIO8, IN1 GPIO9, IN2 GPIO13
- Motor3: PWM GPIO10, IN1 GPIO11, IN2 GPIO15
- Motor4: PWM GPIO14, IN1 GPIO16, IN2 GPIO17

### 비상 스위치
- Emergency Switch: GPIO22 (active-low, internal pull-up)
- 스위치 배선: GPIO22 <-> 스위치 <-> GND

---

## 3) 모터 제어 규칙

### 방향 명령(Dir)
- `FW`: 정회전
- `BW`: 역회전
- `Stop`: 정지
- `Brake`: 브레이크(IN1=HIGH, IN2=HIGH)

### 속도 명령(Speed)
- 정수 `0~100`
- 퍼센트 기반으로 PWM 듀티 변환

### DRV8871 동작
- IN1=HIGH, IN2=LOW -> Forward
- IN1=LOW, IN2=HIGH -> Backward
- IN1=LOW, IN2=LOW -> Coast Stop
- IN1=HIGH, IN2=HIGH -> Brake

---

## 4) MQTT 설정값

### 브로커
- Broker IP: `192.168.0.24`
- Port: `1883`
- Username: `farmmain`
- Password: `eerrtt`

### Last Will
- LWT Topic: `Cart/1/Status`
- LWT Message: `Off Line`

### Client ID
- RP2040 고유 ID 기반
- 형식: `rp2040-<unique_hex>`

---

## 5) MQTT 토픽 구조

### (A) 제어 토픽 (Subscribe)
- `Cart/1/Motor1/Dir`
- `Cart/1/Motor1/Speed`
- `Cart/1/Motor2/Dir`
- `Cart/1/Motor2/Speed`
- `Cart/1/Motor3/Dir`
- `Cart/1/Motor3/Speed`
- `Cart/1/Motor4/Dir`
- `Cart/1/Motor4/Speed`

### (B) 모터 상태 토픽 (Publish)
- `Cart/1/Motor1/StateDir`
- `Cart/1/Motor1/StateSpeed`
- `Cart/1/Motor2/StateDir`
- `Cart/1/Motor2/StateSpeed`
- `Cart/1/Motor3/StateDir`
- `Cart/1/Motor3/StateSpeed`
- `Cart/1/Motor4/StateDir`
- `Cart/1/Motor4/StateSpeed`

값 예시:
- `StateDir`: `FW`, `BW`, `Stop`
- `StateSpeed`: `0`~`100`

### (C) 비상 상태 토픽 (Publish)
- `Cart/1/EmergencySwitch`
  - `Pressed` / `Released`
- `Cart/1/EmergencyLatch`
  - `Latched` / `Normal`

### (D) 디바이스 상태 토픽 (Publish + LWT)
- `Cart/1/Status`
  - 정상 연결 후: `Online`
  - 비상 발생 시: `Emergency`
  - 비정상 종료/LWT: `Off Line`

---

## 6) 부팅 시퀀스

1. `stdio_init_all()`
2. 모터 드라이버 초기화
3. 비상 스위치 인터럽트 초기화
4. ESP01 모듈 초기화(`Esp01Module`)
   - UART 설정
   - 하드웨어 리셋 수행
5. Wi-Fi 연결
6. MQTT 연결(`MqttClient`)
7. 제어 토픽 구독
8. `Cart/1/Status = Online` 발행
9. 명령 대기 루프 진입

---

## 7) 비상 스위치 동작

### 인터럽트
- GPIO22 falling edge 감지
- 디바운스: 50ms

### 비상 발생
- 모든 모터 즉시 비상 정지(브레이크)
- 라치 상태 진입
- `Cart/1/Status = Emergency` 발행
- `EmergencySwitch/EmergencyLatch` 상태 발행

### 해제 후 안전 재가동
- 스위치 해제 상태가 1000ms 연속 안정되면:
  - 모터 안전 정지
  - 모터 드라이버 재초기화
  - 라치 해제
  - `Cart/1/Status = Online` 재발행

중요: 비상 해제 직후 이전 프로세스를 중간부터 이어가지 않고, 안전하게 재가동 경로로 복귀합니다.

---

## 8) 상태 발행 정책

- 이벤트 기반 즉시 발행
  - Dir/Speed 명령 반영 시
  - Brake 적용 시
  - Emergency/Rearm 시
- 주기 발행
  - 1초마다 모터 상태 토픽 재발행
- 비상 상태 토픽
  - 변화 감지 시 즉시 발행

---

## 9) 빌드

사전 조건:
- Pico SDK 설치
- `PICO_SDK_PATH` 환경변수 설정

예시:

```bash
cd motorCartController
mkdir -p build
cd build
cmake ..
make -j
```

산출물:
- `.uf2`
- `.elf`

---

## 10) 배선 체크 포인트

- ESP01 전원은 안정적인 3.3V 공급 사용
- ESP01 `GPIO0/GPIO2/CH_PD`는 기본 부팅 조건에 맞게 pull-up 유지
- RP2040, ESP01, DRV8871 GND 공통 접지
- 모터 전원 라인에 노이즈 대책(벌크 캐패시터, 배선 길이 최소화) 권장

---

## 11) 현재 구현 범위

구현 완료:
- ESP01 초기화/리셋/Wi-Fi 연결
- MQTT 연결 및 제어 토픽 구독
- 4개 모터 개별 제어(Dir/Speed)
- 모터 상태 발행
- 비상 스위치 인터럽트 및 안전 재가동
- 디바이스 상태(`Cart/1/Status`) 발행 + LWT 연동

미구현(추후 확장 가능):
- 상위 애플리케이션 명령 스키마(예: 시나리오 제어)
- 토픽 ACL/보안 강화
- OTA/원격 펌웨어 업데이트

---

## 12) MQTT 테스트 명령 예시 (mosquitto)

아래 예시는 Linux 터미널 기준입니다.

브로커 공통 옵션:

```bash
-h 192.168.0.24 -p 1883 -u farmmain -P eerrtt
```

### 상태 모니터링 (subscribe)

전체 Cart/1 토픽 모니터링:

```bash
mosquitto_sub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/#' -v
```

디바이스 상태만 모니터링:

```bash
mosquitto_sub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Status' -v
```

모터 상태만 모니터링:

```bash
mosquitto_sub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor+/State+' -v
```

비상 상태만 모니터링:

```bash
mosquitto_sub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Emergency+' -v
```

### 모터 제어 (publish)

Motor1 정회전 + 속도 40:

```bash
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor1/Dir' -m 'FW'
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor1/Speed' -m '40'
```

Motor2 역회전 + 속도 60:

```bash
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor2/Dir' -m 'BW'
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor2/Speed' -m '60'
```

Motor3 정지:

```bash
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor3/Dir' -m 'Stop'
```

Motor4 브레이크:

```bash
mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor4/Dir' -m 'Brake'
```

### 자주 쓰는 시나리오

모든 모터 정지:

```bash
for i in 1 2 3 4; do
  mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t "Cart/1/Motor${i}/Dir" -m 'Stop'
done
```

모든 모터 정회전 30:

```bash
for i in 1 2 3 4; do
  mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t "Cart/1/Motor${i}/Dir" -m 'FW'
  mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t "Cart/1/Motor${i}/Speed" -m '30'
done
```

### 참고

- `Dir`의 유효값: `FW`, `BW`, `Stop`, `Brake`
- `Speed`의 유효값: `0`~`100` (정수)
- 디바이스가 비정상 종료되면 `Cart/1/Status`에 LWT 메시지 `Off Line`이 표시됩니다.

---

## 13) tmux 2분할 실습 (실시간 모니터 + 제어)

의도:
- 왼쪽 창에서 상태 토픽을 계속 구독
- 오른쪽 창에서 제어 명령을 보내며 즉시 반응 확인

### tmux 설치 (Linux 예시)

Ubuntu/Debian:

```bash
sudo apt update && sudo apt install -y tmux
```

Fedora:

```bash
sudo dnf install -y tmux
```

Arch:

```bash
sudo pacman -S --noconfirm tmux
```

### 2분할 테스트 순서

1. 새 tmux 세션 시작

```bash
tmux new -s carttest
```

2. 세로 분할 (좌/우 창)

```bash
tmux split-window -h
```

3. 왼쪽 창(0번 pane)에서 전체 상태 구독 실행

```bash
tmux select-pane -t 0
tmux send-keys "mosquitto_sub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/#' -v" C-m
```

4. 오른쪽 창(1번 pane)에서 제어 명령 순차 실행

```bash
tmux select-pane -t 1
tmux send-keys "mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor1/Dir' -m 'FW'" C-m
tmux send-keys "mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor1/Speed' -m '35'" C-m
tmux send-keys "mosquitto_pub -h 192.168.0.24 -p 1883 -u farmmain -P eerrtt -t 'Cart/1/Motor1/Dir' -m 'Brake'" C-m
```

5. 종료

```bash
exit
```

또는 tmux 세션 강제 종료:

```bash
tmux kill-session -t carttest
```

### 기대 결과

- `Cart/1/Motor1/StateDir`, `Cart/1/Motor1/StateSpeed`가 즉시 변함
- 비상 스위치 입력 시 `Cart/1/EmergencySwitch`, `Cart/1/EmergencyLatch` 변경
- 디바이스 상태 `Cart/1/Status`가 `Online/Emergency/Off Line`로 반영

### tmux 없이 테스트하는 방법

- 터미널 2개를 직접 열어서:
  - 터미널 A: `mosquitto_sub ... -t 'Cart/1/#' -v`
  - 터미널 B: `mosquitto_pub ...`
