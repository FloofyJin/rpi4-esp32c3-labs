#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define ADC_CHANNEL ADC_CHANNEL_0
#define DEFAULT_VREF 1100  // Default reference voltage (in millivolts) for ADC

void app_main() {
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    while (1) {
        // Read the voltage from the ADC
        int val = adc1_get_raw(ADC_CHANNEL);

        // Print the result
        printf("Voltage: %d\n", val);

        vTaskDelay(pdMS_TO_TICKS(100));  // Delay for 1 second
    }
}
