#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#define ICM42670_ADDR 0x68
#define I2C_PORT I2C_NUM_0
#define SDA_PIN 10
#define SCL_PIN 8

// static const char *TAG = "lec3";

void i2c_master_init(void){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));
}

void i2c_write(uint8_t reg, uint8_t data){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

uint8_t i2c_read(uint8_t reg){
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return data;
}

void icm42670_init(){
    i2c_write(0x1F, 0b00011111);
}

void read_gyro() {
    int16_t gyro_x, gyro_y, gyro_z;
    gyro_x = (i2c_read(0x11) << 8) | i2c_read(0x12);
    gyro_y = (i2c_read(0x13) << 8) | i2c_read(0x14);
    gyro_z = (i2c_read(0x15) << 8) | i2c_read(0x16);

    float dps_conversion = 2000.0 / 32768.0;

    float gyro_x_dps = (float)gyro_x * dps_conversion;
    float gyro_y_dps = (float)gyro_y * dps_conversion;
    float gyro_z_dps = (float)gyro_z * dps_conversion;

    printf("gyro x: %.2d\n", gyro_x);

    printf("Gyro X: %.2f dps, Gyro Y: %.2f dp, gyro Z: %.2f dps\n", gyro_x_dps, gyro_y_dps, gyro_z_dps);
}

void app_main(void)
{
    i2c_master_init();
    icm42670_init();
    while (1){
        read_gyro();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
