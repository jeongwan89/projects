# RP2040 Blink LED

Raspberry Pi RP2040 (Pico)에서 LED를 깜빡이는 프로젝트입니다.

## 하드웨어 요구사항

- Raspberry Pi Pico (또는 Pico W) 보드
- Micro USB 케이블 (데이터 전송 지원)

## 개발 환경

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- CMake 3.13 이상
- ARM GCC 툴체인 (`arm-none-eabi-gcc`)

## 빌드 방법

```bash
# Pico SDK 경로 설정
export PICO_SDK_PATH=/path/to/pico-sdk

# 빌드 디렉터리 생성
mkdir build && cd build

# CMake 구성
cmake ..

# 빌드
make -j4
```

빌드가 완료되면 `build/blink_led.uf2` 파일이 생성됩니다.

## 플래시 방법

1. **BOOTSEL 버튼을 누른 채로** USB 케이블을 연결합니다.
2. 보드가 `RPI-RP2` 라는 이름의 USB 드라이브로 마운트됩니다.
3. `blink_led.uf2` 파일을 드라이브에 복사합니다.
4. 보드가 자동으로 재부팅되고 LED가 깜빡이기 시작합니다.

## ⚠️ RP2040이 마운트되지 않을 때 (BOOTSEL 모드 진입 방법)

여러 번 플래시 후 RP2040이 USB 드라이브로 마운트되지 않는 경우, 업로드한 펌웨어가 USB 스택을 초기화하기 전에 크래시하거나 USB 연결을 막고 있을 수 있습니다.

### 해결 방법

**방법 1: BOOTSEL 버튼 + 전원 재연결 (권장)**

1. USB 케이블을 분리합니다.
2. 보드의 **BOOTSEL 버튼을 누른 채로** USB 케이블을 다시 연결합니다.
3. 버튼을 떼면 `RPI-RP2` 드라이브가 나타납니다.

**방법 2: BOOTSEL + RUN(RESET) 버튼 동시 사용**

USB가 이미 연결된 상태에서:
1. **BOOTSEL 버튼을 누른 채로** RUN(RESET) 버튼을 짧게 눌렀다가 뗍니다.
2. 그 후 BOOTSEL 버튼을 뗍니다.
3. `RPI-RP2` 드라이브가 나타납니다.

**방법 3: picotool로 강제 재부팅 (USB 인식 시)**

```bash
picotool reboot -f -u
```

### 예방 방법

- `sleep_ms()` 또는 `busy_wait_ms()`를 `main()` 시작 부분에 추가하여 부트로더 진입 시간을 확보합니다:
  ```cpp
  int main() {
      sleep_ms(2000);  // 2초 대기 (BOOTSEL 버튼 누를 시간 확보)
      // ... 이후 코드
  }
  ```
- 또는 `stdio_init_all()` 전에 짧은 대기를 추가합니다.

## 핀 설정

| 핀 | 기능 |
|---|---|
| GPIO25 | 내장 LED (Pico 보드 기준) |

> Pico W의 경우 내장 LED는 `CYW43_WL_GPIO_LED_PIN`으로 제어합니다.
