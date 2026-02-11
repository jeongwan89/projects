#ifndef _ESP07_Read_HM100_MQTT_ver2_
#define _ESP07_Read_HM100_MQTT_ver2_

#pragma once

#include <EspMQTTClient.h>
#include <SoftwareSerial.h>
#include <iostream>
#include <string>

/*
  아래에서 MQTT Topic의 구조를 유지한다.
  Sensor/Topos1/Topos2/Name/Stat
  Actuator/Topos1/Topos2/Name/Act
 */

/*
  #define구문을 정의한다.
  동마다 위치마다 MQTT의 publish가 다르기 때문에 여기에 한번에 걸쳐서 MQTT의 구조를 유지한다.
  장점으로는 파일을 여러개 만들지 않아도 된다.
  단점으로는 컴파일할 때 define의 정의를 명백히 고쳐서 컴파일 해야한다.
  define의 정의를 고쳐서 컴파일하는 일을 컴파일 인자를 고쳐 컴파일한다고 하겠다.
  #define구문의 정의는 Esp07_HM100_MQTT_01 ~ xx (100개)로 하고 각 define의 인자를 정의하는 것으로 한다.
*/

#define Esp07_HM100_MQTT_03

#ifdef Esp07_HM100_MQTT_01
#define CLIENT_NAME "Green_House_HM100_Monitor_PCB_01"
#define MQTT_PUB_TEMP_DRAIN "Sensor/GH1/Rear/Temp_Drain" // GH1 = Green House (온실) 1동
#define MQTT_PUB_EC "Sensor/GH1/Rear/EC"
#define MQTT_PUB_PH "Sensor/GH1/Rear/PH"
#define SENSOR_STATUS "Sensor/GH1/Rear/Stat"
#define MQTT_PUB_AIR_TEMP "Sensor/GH1/Rear/Air_Temp"
#define MQTT_PUB_HUM "Sensor/GH1/Rear/Hum"
#define TEMP_DRAIN_CAL 0
#define EC_CAL 0

#elif defined(Esp07_HM100_MQTT_02)
#define CLIENT_NAME "Green_House_HM100_Monitor_PCB_02"
#define MQTT_PUB_TEMP_DRAIN "Sensor/GH2/Rear/Temp_Drain"
#define MQTT_PUB_EC "Sensor/GH2/Rear/EC"
#define MQTT_PUB_PH "Sensor/GH2/Rear/PH"
#define SENSOR_STATUS "Sensor/GH2/Rear/Stat"
#define MQTT_PUB_AIR_TEMP "Sensor/GH2/Rear/Air_Temp"
#define MQTT_PUB_HUM "Sensor/GH2/Rear/Hum"
#define TEMP_DRAIN_CAL 0
#define EC_CAL 0

#elif defined(Esp07_HM100_MQTT_03)
#define CLIENT_NAME "Green_House_HM100_Monitor_PCB_03"
#define MQTT_PUB_TEMP_DRAIN "Sensor/GH3/Rear/Temp_Drain"
#define MQTT_PUB_EC "Sensor/GH3/Rear/EC"
#define MQTT_PUB_PH "Sensor/GH3/Rear/PH"
#define SENSOR_STATUS "Sensor/GH3/Rear/Stat"
#define MQTT_PUB_AIR_TEMP "Sensor/GH3/Rear/Air_Temp"
#define MQTT_PUB_HUM "Sensor/GH3/Rear/Hum"
#define TEMP_DRAIN_CAL 0
#define EC_CAL 0

#elif defined(Esp07_HM100_MQTT_04)
#define CLIENT_NAME "Green_House_HM100_Monitor_PCB_04"
#define MQTT_PUB_TEMP_DRAIN "Sensor/GH4/Rear/Temp_Drain"
#define MQTT_PUB_EC "Sensor/GH4/Rear/EC"
#define MQTT_PUB_PH "Sensor/GH4/Rear/PH"
#define SENSOR_STATUS "Sensor/GH4/Rear/Stat"
#define MQTT_PUB_AIR_TEMP "Sensor/GH4/Rear/Air_Temp"
#define MQTT_PUB_HUM "Sensor/GH4/Rear/Hum"
#define TEMP_DRAIN_CAL 0
#define EC_CAL 0

#elif defined(Esp07_HM100_MQTT_05)
#define CLIENT_NAME "NR_House_HM100_Monitor_PCB_05"
#define MQTT_PUB_TEMP_DRAIN "Sensor/NR1/Rear/Temp_Drain" // NR1 = Nursery Green House (육묘장) 1동
#define MQTT_PUB_EC "Sensor/NR1/Rear/EC"
#define MQTT_PUB_PH "Sensor/NR1/Rear/PH"
#define SENSOR_STATUS "Sensor/NR1/Rear/Stat"
#define MQTT_PUB_AIR_TEMP "Sensor/NR1/Rear/Air_Temp"
#define MQTT_PUB_HUM "Sensor/NR1/Rear/Hum"
#define TEMP_DRAIN_CAL 0
#define EC_CAL 0
#endif

// Pin 정의
#define SSerialTxControl 16
#define RS485RX 13  // PCB 설계 단계에서 선이 꼬여서 핀 번호를 바꿨음
#define RS485TX 12
#define LED_PIN 2 // ESP-12 built-in LED is on GPIO2

// 함수 정의
#define CHIP485_SEL_TX digitalWrite(SSerialTxControl, HIGH) // Transmission 485에서 외부로 전송
#define CHIP485_SEL_RX digitalWrite(SSerialTxControl, LOW)  // Receive 485가 외부에서 전입

// 상수 정의
#define REFRESH_TIME 10 // sec(초단위) REFRESH_TIME 마다 온도/습도를 읽어서 MQTT에 올리기 위한 인터벌 시간
#define READ_INTERVAL_MS 5000 // RS485 읽기 간격 (밀리초)
#define RS485_RESPONSE_WAIT_MS 200 // RS485 응답 대기 시간 (밀리초)
#define DATA_BUFFER_SIZE 16 // 수신 데이터 버퍼 크기
#define MODBUS_REQUEST_SIZE 8 // Modbus 요청 데이터 크기
#define MODBUS_RESPONSE_SIZE 15 // 예상되는 Modbus 응답 크기
#define EC_SCALE_FACTOR 100.0f // EC 값 스케일 팩터
#define PH_SCALE_FACTOR 100.0f // pH 값 스케일 팩터
#define TEMP_SCALE_FACTOR 10.0f // 온도 값 스케일 팩터
#define WORD_HIGH_BYTE_SHIFT 256 // 워드 값 상위 바이트 시프트
#define ENABLE_LED_DIAG 1 // 부팅 시 LED 진단 패턴 실행
#define DEBUG_RS485 1 // RS485 디버그 로그 활성화

// 
extern EspMQTTClient client;
extern SoftwareSerial rs485;
extern float EC;
extern float pH;
extern float temp_drain;

// extern unsigned char Data[16];
// extern byte requestData[9];

// mqtt server에 연결되었을 때 작동하는 event 함수
void onConnectionEstablished(void);

// HM-100에 있는 Registry에 데이터를 요구하는 함수
// 전역변수로 인자를 입출력 받는다.
void DemandData(void);

// HM-100이 응답하여 오는 신호를 SoftwareSerial에 받는 함수
// 전역변수에 저장
int ReadData(void);

// Modbus CRC16 검증 함수
// 인수: data - 검증할 데이터 배열, length - 데이터 길이
// 반환: true - CRC 정상, false - CRC 오류
bool verifyCRC(unsigned char* data, int length);

// 받아온 전역변수 데이터를 사람이 알기 쉽게 변환함
void Parsing(void);

// RaiseTimeEventInLoop에 사용할 함수
void read485InClass(void);

// ESP-12 built-in LED(GPIO2)에 깜빡이는 신호
// 인수 : 0.1sec 인터벌로 깜빡임 LOW는 점등, HIGH는 꺼짐
void blink(int);

// MQTT 서버에 퍼블리쉬
// 이 전의 값을 가지고 있다가 다르면 실제로 publish
int publishValue(void);

#endif