# RP2040 MQTT 프로젝트 구조 요약

## 주요 구조

- `components/` : WiFi/MQTT, 센서, 액츄에이터 등 재사용 가능한 모듈 집합
- `projects/`   : 실제 애플리케이션(예: connectBroker, temp_monitor 등)
- 각 모듈은 `inc/`, `src/`, `CMakeLists.txt`로 구성
- 공통 로그(debug_log)는 wifi_mqtt 모듈에 통합, 모든 프로젝트에서 재사용

## 예시 디렉토리

```
components/
  wifi_mqtt/
    inc/uart_comm.h, esp01.h, mqtt_client.h, debug_log.h, serial_bridge.h
    src/uart_comm.c, esp01.c, mqtt_client.c, debug_log.c, serial_bridge.c
  sensors/
    dht22/, bmp280/, adc_sensor/, ds18b20/ (각각 inc, src, CMakeLists.txt)
  actuators/
    relay/, servo/, pwm_led/, motor/ (각각 inc, src, CMakeLists.txt)
projects/
  connectBroker/
    src/main.c
    inc/config.h
    CMakeLists.txt
```

## 모듈 사용법

- 프로젝트 CMakeLists.txt에서 필요한 모듈을 add_subdirectory, target_link_libraries로 연결
- 예시:
  ```cmake
  add_subdirectory(${CMAKE_SOURCE_DIR}/../../components/wifi_mqtt wifi_mqtt)
  target_link_libraries(${PROJECT_NAME} wifi_mqtt)
  ```

## 장점

- 모듈화, 재사용성, 유지보수성, 확장성, 독립 테스트 용이

## 참고

- 상세 구조 및 예시는 ARCHITECTURE.md 참고
