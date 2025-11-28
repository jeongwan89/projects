# TM1637 테스트 프로그램

이 디렉토리는 TM1637 7-세그먼트 디스플레이의 독립적인 테스트를 위한 프로그램을 포함합니다.

## 파일 구조

- `main.cpp` - TM1637 테스트 메인 프로그램
- `CMakeLists.txt` - 실행 파일 빌드 설정
- `CMakeLists.txt.backup_lib` - 원본 라이브러리 빌드 설정 (백업)
- `build.sh` - 빌드 스크립트
- `inc/` - 헤더 파일
- `src/` - 소스 파일

## 하드웨어 연결

기본 핀 설정 (main.cpp에서 변경 가능):
- CLK: GPIO 6
- DIO: GPIO 7
- VCC: 3.3V 또는 5V
- GND: GND

## 빌드 방법

```bash
./build.sh
```

또는 수동 빌드:
```bash
mkdir -p build
cd build
cmake ..
make -j4
```

## 업로드 방법

1. RP2040의 BOOTSEL 버튼을 누른 채로 USB 연결
2. 다음 명령 실행:
```bash
cp build/tm1637_test.uf2 /media/$USER/RPI-RP2/
```

## 테스트 항목

프로그램은 다음 테스트를 순차적으로 수행합니다:

1. **초기화 테스트** - TM1637 통신 확인
2. **전체 세그먼트 테스트** - 8888 표시
3. **화면 클리어 테스트**
4. **숫자 카운트** - 0부터 99까지
5. **소수점 숫자 표시** - 12.3
6. **온도 표시** - 25.6°C
7. **습도 표시** - 65.5%
8. **밝기 변경** - 0부터 7까지
9. **ON/OFF 토글** - 5회 반복
10. **개별 세그먼트 테스트** - 각 자리 순차 표시

이후 무한 루프로 온도/습도 시뮬레이션을 번갈아 표시합니다.

## 시리얼 모니터

USB CDC를 통해 테스트 진행 상황을 실시간으로 확인할 수 있습니다:

```bash
# minicom 사용
minicom -b 115200 -D /dev/ttyACM0

# screen 사용
screen /dev/ttyACM0 115200
```

## 문제 해결

### 디스플레이가 작동하지 않는 경우

1. **하드웨어 연결 확인**
   - CLK, DIO 핀이 올바르게 연결되었는지 확인
   - VCC/GND 연결 확인
   - 케이블 불량 확인

2. **핀 번호 확인**
   - `main.cpp`의 `TEST_CLK_PIN`, `TEST_DIO_PIN` 확인
   - 실제 연결과 일치하는지 확인

3. **전원 확인**
   - 3.3V 또는 5V 전원이 정상적으로 공급되는지 확인
   - 전류가 충분한지 확인 (밝기 7에서 최대 약 80mA)

4. **시리얼 모니터로 디버깅**
   - 초기화 실패 메시지 확인
   - ACK 신호 수신 여부 확인

## 라이브러리로 사용

다른 프로젝트에서 TM1637을 라이브러리로 사용하려면:

```bash
# 백업된 라이브러리 CMakeLists.txt 복원
cp CMakeLists.txt.backup_lib CMakeLists.txt
```

그리고 상위 프로젝트의 CMakeLists.txt에서:
```cmake
add_subdirectory(path/to/tm1637 tm1637)
target_link_libraries(your_project tm1637)
```
