#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include "esp_timer.h"

#define TRIGGER_GPIO 0
#define ECHO_GPIO 1
#define MAX_DISTANCE_CM 400
#define TIMEOUT_US (MAX_DISTANCE_CM*58)

void ultrasonic_task(void *pvParameters){
  while (true){
    gpio_set_level(TRIGGER_GPIO, 0);
    
    esp_rom_delay_us(2);
    gpio_set_level(TRIGGER_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIGGER_GPIO, 0);

    uint32_t start = esp_timer_get_time();
    while(gpio_get_level(ECHO_GPIO) == 0 && esp_timer_get_time() - start < TIMEOUT_US);

    start = esp_timer_get_time();
    while(gpio_get_level(ECHO_GPIO) == 1 && esp_timer_get_time() - start < TIMEOUT_US);

    uint32_t distance = (esp_timer_get_time() - start) / 58;

    printf("Distance %ld cm\n", distance);

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

void app_main(){

  ultrasonic_init();

  xTaskCreate(ultrasonic_task, "ultrasonic_task", 2048, NULL, 5, NULL);

}
