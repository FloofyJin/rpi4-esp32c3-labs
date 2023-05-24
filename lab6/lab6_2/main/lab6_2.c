#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_timer.h"

#define ADC_CHANNEL ADC_CHANNEL_0
#define DEFAULT_VREF 1100  // Default reference voltage (in millivolts) for ADC
#define DOT_DURATION_MS 200

// Define Morse code symbols and their corresponding English characters
const char *morseSymbols[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
                              "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};
const char englishChars[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
                             'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

// Function to convert Morse code to English
char morseToEnglish(const char *morse) {
    int numSymbols = sizeof(morseSymbols) / sizeof(morseSymbols[0]);
    for (int i = 0; i < numSymbols; i++) {
        if (strcmp(morse, morseSymbols[i]) == 0) {
            return englishChars[i];
        }
    }
    return '?';  // Return '?' if Morse code symbol is not found
}

void app_main() {
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);

    int64_t start_time = esp_timer_get_time();
    bool currentOn = false;
    bool lightOn = false;
    char code[4] = "";
    int i = 0;
    uint64_t diff;

    while (1) {
        // Read the voltage from the ADC
        int val = adc1_get_raw(ADC_CHANNEL);

        // Print the result
        // printf("Voltage: %d\n", val);
        // int64_t currentTime = esp_timer_get_time();
        // printf("Current time: %lld ms\n", currentTime);

        if (val > 50){
            lightOn = true;
        }else{
            lightOn = false;
        }

        if(!currentOn && lightOn){
            diff = esp_timer_get_time() - start_time;
            // if(diff >= 1400000){
            //     printf(" ");
            // }
            currentOn = true;
            start_time = esp_timer_get_time();
        }else if(currentOn && !lightOn ){
            diff = esp_timer_get_time() - start_time;
            if (diff>=590000){ // dash
                code[i++] = '-';
            }else { // dot
                code[i++] = '.';
            }
            currentOn = false;
            start_time = esp_timer_get_time();
            //  printf("word: %c\n", morseToEnglish(code));
        }else if(!currentOn && !lightOn){ // !currentOn && !lightOn
            diff = esp_timer_get_time() - start_time;
            if(code[0] != '\0'){
                if(diff >= 690000){//new letter
                    printf("%c\n", morseToEnglish(code));
                    code[0] = '\0';
                    code[1] = '\0';
                    code[2] = '\0';
                    code[3] = '\0';
                    i = 0;
                }else{//new dot/dash for the current letter
                    // do nothing
                }
            }
        }else{ //currentOn && lightOn
            // do nothing
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Delay for 1 second
    }
}
