# DS18B20 센서 RP2040 예제

- DS18B20 1-Wire 온도 센서 예제입니다.
- 기본 연결: 데이터핀 → GPIO16
- 실제 온도 읽기는 1-Wire 프로토콜 구현 필요(여기서는 핀 토글 예시)

## 빌드 및 실행
```bash
mkdir build && cd build
cmake ..
make -j
```
생성된 `ds18b20_example.uf2`를 RP2040에 복사하면 USB CDC로 핀 토글 상태가 출력됩니다.
