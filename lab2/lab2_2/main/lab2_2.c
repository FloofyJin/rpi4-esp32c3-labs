#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "lec2";

#define I2C_MASTER_SCL_IO 8 /*!< GPIO number for I2C master SCL */
#define I2C_MASTER_SDA_IO 10 /*!< GPIO number for I2C master SDA */
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_FREQ_HZ 100000
#define SHTC3_SENSOR_ADDR 0x70 /*!< SHTC3 12C address */

#define SHTC3_CMD_READ_ID OxEFCS8
//#define SHTC3_CMD_MEASURE 0x7866
#define SHTC3_CMD_MEASURE 0x7CA2

static esp_err_t i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to i2c_param_config %d", err);
        return err;
    }

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE,0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to i2c_driver_install %d", err);
        return err;
    }
    return err;
}

static esp_err_t shtc3_read(uint16_t command, uint8_t *data, size_t size){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    esp_err_t err;
    #if 1
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

    #endif
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

void shtc3_task(){

    // temperature_sensor_handle_t temp_handle = NULL;
    // temperature_sensor_config_t temp_sensor = {
    //     .range_min = 20,
    //     .range_max = 50,
    //     .clk_src = TEMPERATURE_SENSOR_CLK_SRC_XTAL,
    // };
    // ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
    // ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
    // Get converted sensor data
    // float tsens_out;
    // Disable the temperature sensor if it's not needed and save the power
    // ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));

    while(1){

        uint8_t data[6] = {0,};
        uint16_t raw_humidity=0;
        uint16_t raw_temperature=0;
        // ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));

        esp_err_t err = shtc3_read(SHTC3_CMD_MEASURE, data, 6);
        if(err == ESP_OK){
            raw_humidity = (data[3] << 8) | data[4];
            raw_temperature = (data[0] << 8) | data[1];
            float humidity = calculate_humidity(raw_humidity);
            float temperature = calculate_temperature(raw_temperature);
            ESP_LOGI(TAG, "Humidity: %.2f %%", humidity);
            ESP_LOGI(TAG, "Temperature: %.2f C (%.2f F)", temperature, (temperature*1.8)+32);
        } else {
            ESP_LOGI(TAG, "Failed to read data from SHTC3 sensor %d", err);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void app_main(void){
    esp_err_t err = i2c_master_init();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master");
        return;
    }

    shtc3_task();
}   