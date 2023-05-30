#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include "esp_timer.h"
#include <math.h>

#include "driver/i2c.h"

// distance 
#define TRIGGER_GPIO 0
#define ECHO_GPIO 1
#define MAX_DISTANCE_CM 400
#define TIMEOUT_US (MAX_DISTANCE_CM*58)

// temperature
#define I2C_MASTER_SCL_IO 8 /*!< GPIO number for I2C master SCL */
#define I2C_MASTER_SDA_IO 10 /*!< GPIO number for I2C master SDA */
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_FREQ_HZ 100000
#define SHTC3_SENSOR_ADDR 0x70 /*!< SHTC3 12C address */
#define SHTC3_CMD_MEASURE 0x7CA2

static const char *TAG = "distance sensor";

float temperature;

void ultrasonic_task(void *pvParameters){
  while (true){
    gpio_set_level(TRIGGER_GPIO, 0);
    
    esp_rom_delay_us(2);
    gpio_set_level(TRIGGER_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIGGER_GPIO, 0);

    //wait for sensor to send wave
    uint32_t start = esp_timer_get_time();
    while(gpio_get_level(ECHO_GPIO) == 0 && esp_timer_get_time() - start < TIMEOUT_US);

    //wait for sensor to receive wave
    start = esp_timer_get_time();
    while(gpio_get_level(ECHO_GPIO) == 1 && esp_timer_get_time() - start < TIMEOUT_US);

    //27 === room temperature
    //esp_timer_get_time() is in us
    //
    // uint32_t distance = (esp_timer_get_time() - start) / 58;
    float distance = (esp_timer_get_time() - start) * (331.4+0.605*sqrt(temperature+273))*100 * 0.000001 / 2;

    ESP_LOGI(TAG, "Distance %0.2f cm at %.2f C (%.2f F)", distance, temperature, (temperature*1.8)+32);

    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void ultrasonic_init(){
  gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 1ULL << TRIGGER_GPIO,
  };
  gpio_config(&io_conf);

  io_conf.intr_type = GPIO_INTR_POSEDGE,
  io_conf.mode = GPIO_MODE_INPUT,
  io_conf.pin_bit_mask = 1ULL << ECHO_GPIO,
  gpio_config(&io_conf);
}

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

static float calculate_temperature(uint16_t raw_temperature)
{
    return raw_temperature * 175.0 / 65535.0 - 45.0;
}

void shtc3_task(){
    while(1){

        uint8_t data[6] = {0,};
        // uint16_t raw_humidity=0;
        uint16_t raw_temperature=0;

        esp_err_t err = shtc3_read(SHTC3_CMD_MEASURE, data, 6);
        if(err == ESP_OK){
            // raw_humidity = (data[3] << 8) | data[4];
            raw_temperature = (data[0] << 8) | data[1];
            // float humidity = calculate_humidity(raw_humidity);
            temperature = calculate_temperature(raw_temperature);
            // sprintf(esp_hum, "%s%.0f%%", "Hum : ", humidity);
            // sprintf(esp_temp, "%s%.0fC", "Temp: ", temperature);
            // ESP_LOGI(TAG, "Humidity: %.2f %%", humidity);
            // ESP_LOGI(TAG, "Temperature: %.2f C (%.2f F)", temperature, (temperature*1.8)+32);
        } else {
            ESP_LOGI(TAG, "Failed to read data from SHTC3 sensor %d", err);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(){

  ultrasonic_init();

  //Initialize i2c for h and t
  esp_err_t err = i2c_master_init();
  if(err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize I2C master");
      return;
  }

  xTaskCreate(ultrasonic_task, "ultrasonic_task", 2048, NULL, 5, NULL);

  xTaskCreate(&shtc3_task, "shtc3_task", 4096, NULL, 5, NULL);

}
