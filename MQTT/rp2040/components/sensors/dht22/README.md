# DHT22 센서 RP2040 예제

- DHT22 온습도 센서를 Raspberry Pi Pico(RP2040)에서 읽는 예제입니다.
- 기본 연결: DHT22 데이터핀 → GPIO 15

## 빌드 및 실행
```bash
mkdir build && cd build
cmake ..
make -j
```
생성된 `dht22_example.uf2`를 RP2040에 복사하면 USB CDC로 온도/습도 값이 출력됩니다.
