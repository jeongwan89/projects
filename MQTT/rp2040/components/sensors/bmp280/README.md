# BMP280 센서 RP2040 예제

- BMP280 기압/온도 센서를 I2C로 읽는 예제입니다.
- 기본 연결: SDA=GPIO4, SCL=GPIO5

## 빌드 및 실행
```bash
mkdir build && cd build
cmake ..
make -j
```
생성된 `bmp280_example.uf2`를 RP2040에 복사하면 USB CDC로 센서 ID가 출력됩니다.
