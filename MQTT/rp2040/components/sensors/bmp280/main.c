#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define BMP280_ADDR 0x76
#define SDA_PIN 4
#define SCL_PIN 5

// BMP280 레지스터 및 보정 상수 등은 실제 드라이버에 맞게 수정 필요
uint8_t bmp280_read_id() {
    uint8_t id = 0;
    uint8_t reg = 0xD0;
    i2c_write_blocking(I2C_PORT, BMP280_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, BMP280_ADDR, &id, 1, false);
    return id;
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    printf("\n=== BMP280 센서 예제 (RP2040) ===\n");
    sleep_ms(500);
    uint8_t id = bmp280_read_id();
    printf("BMP280 ID: 0x%02X\n", id);
    // 실제 온도/기압 읽기는 드라이버 필요
    while (true) {
        printf("(여기에 온도/기압 읽기 코드 추가)\n");
        sleep_ms(2000);
    }
    return 0;
}
