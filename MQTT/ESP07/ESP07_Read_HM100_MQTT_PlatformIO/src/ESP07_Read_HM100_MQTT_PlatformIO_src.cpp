#include <Arduino.h>
#include "ESP07_Read_HM100_MQTT_PlatformIO_src.h"

unsigned char Data[16];
byte requestData[9] = {
    0x01, // Device ID
    0x03, // Function
    0x00, // Address high byte
    0x68, // Address low byte
    0x00, // Data hight byte
    0x03, // Data low byte
    0x84, // CRC high byte
    0x17  // CRC low byte
};

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
    client.publish(SENSOR_STATUS, (String) "On Line", true);
    // You can activate the retain flag by setting the third parameter to true
    // 아래 코드는 그 이전 프로젝트에서 받은 코드이다.
    // Relay 작동을 하지 않는 프로그램이어서, MQTT-> Actuator 서브스크라이브를 하지 않게 되었다.
    // 그래서 남은 코드의 흔적을 아래에 둔다.
    /*
    client.subscribe("Actuator/HM/Rear/Heater",[](const String & payload){
        if(payload == "1"){
        digitalWrite(RELAY1, HIGH);
        } else {
        digitalWrite(RELAY1, LOW);
        }
    });
    client.subscribe("Actuator/HM/Rear/Fuel",[](const String & payload){
        if(payload == "1"){
        digitalWrite(RELAY2, HIGH);
        } else {
        digitalWrite(RELAY2, LOW);
        }
    });
    */
    /*
        // Execute delayed instructions
        // 아래 코드는 예제에서 소개된 코드로 지정된 시간뒤에 어떤 동작을 하는 코드이다.
        client.executeDelayed(5 * 1000, []() {
            client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
        });
    */
}

void blink(int noRepeat)
{
    digitalWrite(LED_PIN, HIGH);
    for(int i = 0 ; i < noRepeat; i++)
    {
        digitalWrite(LED_PIN, LOW);
        delay(50);
        digitalWrite(LED_PIN, HIGH);
        delay(50);
    }
}

void DemandData(void)
{
    char mP[7];
    CHIP485_SEL_TX;

#if DEBUG_RS485
    Serial.println("RS485: TX start");
#endif

    // 이후에 SoftwareSerial rs485에 byte requestData 써 넣는 루틴
    // rs485.write(requestData,8);
    // 한 바이트씩 보내기
    for (int i = 0; i < MODBUS_REQUEST_SIZE; i++)
    {
        rs485.write(requestData[i]);
    }

    Serial.print("Demand Data to HM-100 sending:");
    for (int i = 0; i < MODBUS_REQUEST_SIZE; i++)
    {
        sprintf(mP, "0x%02x ", requestData[i]);
        Serial.print(mP);
    }
    Serial.println();

    rs485.flush();
    delay(3); // RS485 TX->RX turn-around guard

    CHIP485_SEL_RX;

#if DEBUG_RS485
    Serial.println("RS485: TX done, RX enabled");
#endif
}


int ReadData(void)
{
    int index = 0;
    char mP[7];
    CHIP485_SEL_RX;

    rs485.listen();

#if DEBUG_RS485
    Serial.print("RS485: available before read = ");
    Serial.println(rs485.available());
#endif

    index = rs485.available();
    if (index > 0)
    {
        // 버퍼 오버플로우 방지: 버퍼 크기로 제한
        if (index > DATA_BUFFER_SIZE) {
            Serial.print("Warning: Received data size (");
            Serial.print(index);
            Serial.print(") exceeds buffer size (");
            Serial.print(DATA_BUFFER_SIZE);
            Serial.println("). Truncating data.");
            index = DATA_BUFFER_SIZE;
        }
        
        Serial.print("Received Data:");
        for (int i = 0; i < index; i++)
        {
            Data[i] = rs485.read();
            sprintf(mP, "0x%02x ", Data[i]);
            Serial.print(mP);
        }
        Serial.print("\t");
        Serial.println(index);
        //만약 읽기가 성공이면 blink 1번하기
        blink(1);
    }
#if DEBUG_RS485
    else {
        Serial.println("RS485: no data received");
    }
#endif
    CHIP485_SEL_RX;
    return index;
}

// Modbus RTU CRC16 검증 함수
bool verifyCRC(unsigned char* data, int length)
{
    if (length < 3) return false; // 최소 크기 확인
    
    uint16_t crc = 0xFFFF;
    
    // CRC는 마지막 2바이트를 제외하고 계산
    for (int i = 0; i < length - 2; i++) {
        crc ^= (uint16_t)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    // 계산된 CRC와 수신된 CRC 비교 (Little Endian)
    uint16_t receivedCRC = ((uint16_t)data[length - 1] << 8) | (uint16_t)data[length - 2];
    
    if (crc != receivedCRC) {
        Serial.print("CRC Error! Calculated: 0x");
        Serial.print(crc, HEX);
        Serial.print(", Received: 0x");
        Serial.println(receivedCRC, HEX);
        return false;
    }
    
    return true;
}


void Parsing(void)
{
    // uint16_t로 변환하여 부호 확장 문제 방지
    uint16_t ecRaw = ((uint16_t)Data[3] << 8) | (uint16_t)Data[4];
    uint16_t phRaw = ((uint16_t)Data[5] << 8) | (uint16_t)Data[6];
    uint16_t tempRaw = ((uint16_t)Data[7] << 8) | (uint16_t)Data[8];

    EC = (float)ecRaw / EC_SCALE_FACTOR;
    pH = (float)phRaw / PH_SCALE_FACTOR;
    temp_drain = (float)tempRaw / TEMP_SCALE_FACTOR;

    for (int i = 0; i < DATA_BUFFER_SIZE; i++)
    {
        Data[i] = 0x00;
    }
    // 참고로 pH, temp, EC는 hm-100의 단위 select에 따라 달라진다.
    Serial.print("pH = ");
    Serial.print(pH);
    Serial.print('\t');
    Serial.print("tp = ");
    Serial.print(temp_drain);
    Serial.print('\t');
    Serial.print("EC = ");
    Serial.println(EC);
}

void read485InClass(void)
{
#if DEBUG_RS485
    Serial.println("RS485: cycle start");
#endif
    DemandData();
    delay(RS485_RESPONSE_WAIT_MS);
    
    int receivedBytes = ReadData();
    
    // 데이터 길이 검증
    if(receivedBytes != MODBUS_RESPONSE_SIZE) {
        Serial.print("Error: Expected ");
        Serial.print(MODBUS_RESPONSE_SIZE);
        Serial.print(" bytes, but received ");
        Serial.println(receivedBytes);
#if DEBUG_RS485
        Serial.println("RS485: response length mismatch");
#endif
        return;
    }
    
    // CRC 검증
    if(!verifyCRC(Data, receivedBytes)) {
        Serial.println("Error: CRC verification failed!");
#if DEBUG_RS485
        Serial.println("RS485: CRC failed");
#endif
        return;
    }
    
    Parsing();
    delay(RS485_RESPONSE_WAIT_MS);
    publishValue();

#if DEBUG_RS485
    Serial.println("RS485: cycle end");
#endif
}

int publishValue(void)
{
    static float last_EC = 0;
    static float last_pH = 0;
    static float last_temp_drain = 0;

    if(last_EC != EC)
    {
        char strValue [16];
        snprintf(strValue, sizeof(strValue), "%.2f", EC); 
        if(client.publish(MQTT_PUB_EC, strValue, true))
        {
            last_EC = EC;
            blink(2);
        }
    }

    if(last_pH != pH)
    {
        char strValue [16];
        snprintf(strValue, sizeof(strValue), "%.2f", pH);
        if(client.publish(MQTT_PUB_PH, strValue, true))
        {
            last_pH = pH;
            blink(3);
        }
    }

    if(last_temp_drain != temp_drain)
    {
        char strValue [16];
        snprintf(strValue, sizeof(strValue), "%.1f", temp_drain);
        if(client.publish(MQTT_PUB_TEMP_DRAIN, strValue, true))
        {
            last_temp_drain = temp_drain;
            blink(4);
        }
    }
    return 0;
}