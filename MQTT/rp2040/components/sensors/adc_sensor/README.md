# ADC 센서 RP2040 예제

- Pico의 내장 ADC를 사용한 아날로그 입력 예제입니다.
- 기본 연결: 가변저항 등 아날로그 입력 → GPIO26(ADC0)

## 빌드 및 실행
```bash
mkdir build && cd build
cmake ..
make -j
```
생성된 `adc_sensor_example.uf2`를 RP2040에 복사하면 USB CDC로 ADC 값이 출력됩니다.
