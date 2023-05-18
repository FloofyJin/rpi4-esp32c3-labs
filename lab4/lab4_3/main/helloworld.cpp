#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "DFRobot_LCD.h"
#include "i2c_master_init.h"

extern "C" {
    void app_main(void);
}

static const char *TAG = "lec4";

DFRobot_LCD lcd(16,2);

#define I2C_MASTER_NUM I2C_NUM_0
#define SHTC3_CMD_READ_ID OxEFCS8
#define SHTC3_CMD_MEASURE 0x7CA2
#define SHTC3_SENSOR_ADDR 0x70 

static esp_err_t shtc3_read(uint16_t command, uint8_t *data, size_t size){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    esp_err_t err;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR <<1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, command >> 8, true);
    i2c_master_write_byte(cmd, command & 0xFF, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to 1st write %d", err);

        i2c_cmd_link_delete(cmd);
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);
    
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SHTC3_SENSOR_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    if(err != ESP_OK){
        ESP_LOGE(TAG, "failed to 2nd read %d", err);
    }
    
    i2c_cmd_link_delete(cmd);
    return err;
}

static float calculate_humidity(uint16_t raw_humidity)
{
    return 100.0 * (float)raw_humidity / 65535.0;
}

static float calculate_temperature(uint16_t raw_temperature)
{
    return raw_temperature * 175.0 / 65535.0 - 45.0;
}

void app_main(void)
{
    lcd.init();
    while(1){
        lcd.setRGB(0,255,0);

        uint8_t data[6] = {0,};
        uint16_t raw_humidity=0;
        uint16_t raw_temperature=0;

        char temp[10];
        char hum[10];

        esp_err_t err = shtc3_read(SHTC3_CMD_MEASURE, data, 6);
        if(err == ESP_OK){
            raw_humidity = (data[3] << 8) | data[4];
            raw_temperature = (data[0] << 8) | data[1];
            float humidity = calculate_humidity(raw_humidity);
            float temperature = calculate_temperature(raw_temperature);
            sprintf(temp, "%s%.0fC", "Temp: ", temperature);
            sprintf(hum, "%s%.0f%%", "Hum : ", humidity);
            lcd.setCursor(0,0);
            lcd.printstr(temp);
            lcd.setCursor(0,1);
            lcd.printstr(hum);
            } else {
            ESP_LOGI(TAG, "Failed to read data from SHTC3 sensor %d", err);
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
