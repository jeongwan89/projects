/*
    //SimpleMQTTClient.ino
    The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
    Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
    It will also send a message delayed 5 seconds later.

    ESP07 -> ESO07_Example.ino -> ESP07_Read_HM100.ino -> ESP07_Read_HM100_MQTT.ino
    -> ESP01_Read_HM100_MQTT_ver2.ino
    이 코드의 목적은 HM100에서 EC pH Temp_drain를 RS485에서 읽고
    그 온도 값을 MQTT서버에 올리는 일이다.
*/
#include <Arduino.h>
#include <EspMQTTClient.h>
#include <SoftwareSerial.h>

#include "ESP07_Read_HM100_MQTT_PlatformIO_src.h"
#include "credentials.h"
#include <RaiseEventClass.h>

// 
EspMQTTClient client(
    WIFI_SSID,
    WIFI_PASSWORD,
    MQTT_BROKER_IP, // MQTT Broker server ip
    MQTT_USERNAME,  // Can be omitted if not needed
    MQTT_PASSWORD,  // Can be omitted if not needed
    CLIENT_NAME,    // Client name that uniquely identify your device
    MQTT_PORT       // The MQTT port, default to 1883. this line can be omitted
);

SoftwareSerial rs485(RS485RX, RS485TX);
float EC;
float pH;
float temp_drain;
RaiseTimeEventInLoop read485;

// extern char Data[16];

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(SSerialTxControl, OUTPUT);

    Serial.begin(115200);
    rs485.begin(19200);

    blink(2);
    CHIP485_SEL_RX;

    // Optional functionalities of EspMQTTClient
    // Enable debugging messages sent to serial output
    client.enableDebuggingMessages();

    // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword.
    // These can be overridded with enableHTTPWebUpdater("user", "password").
    client.enableHTTPWebUpdater();

    // Enable OTA (Over The Air) updates.
    // Password defaults to MQTTPassword. Port is the default OTA port.
    // Can be overridden with enableOTA("password", port).
    client.enableOTA();

    // You can activate the retain flag by setting the third parameter to true
    client.enableLastWillMessage(SENSOR_STATUS, "Fail", true);

}

void loop()
{
    client.loop();
    read485.EachEveryTimeIn(READ_INTERVAL_MS, read485InClass);
}
