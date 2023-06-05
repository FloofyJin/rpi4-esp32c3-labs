#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_timer.h"

#define ADC_CHANNEL ADC_CHANNEL_0
#define DEFAULT_VREF 1100  // Default reference voltage (in millivolts) for ADC

// mess around with the values to make it work
#define COUNT_TIME 10 // how fast counter should update
#define DOT_COUNT 2 
#define DASH_COUNT DOT_COUNT*3
#define SPACE_COUNT DOT_COUNT*4
#define LIGHT_THRESHOLD 50

// Define Morse code symbols and their corresponding English characters
const char *morseSymbols[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
                           "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
                           "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};
const char englishChars[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
                             'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

// Function to convert Morse code to English
char morseToEnglish(const char *morse) {
    // printf("morse: %s", morse);
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

    bool currentOn = false;
    bool lightOn = false;
    char code[6] = "";
    int i = 0;

    // just has to be a big number. so you dont print random char at the beginning
    int counter = SPACE_COUNT+DASH_COUNT;

    while (1) {
        // Read the voltage from the ADC
        int val = adc1_get_raw(ADC_CHANNEL);
        if (val > LIGHT_THRESHOLD){
            lightOn = true;
        }else{
            lightOn = false;
        }

        if(currentOn && !lightOn){ // light turns off
            if(counter >= DASH_COUNT){
                code[i++] = '-'; // dash
            }else {
                code[i++] = '.'; // dot
            }
            counter = 0;
            currentOn = false;
        }else if(!currentOn && lightOn){ // light turns on
            counter = 0;
            currentOn = true;
        }if(!currentOn){ // light is currently off
            if(code[0] != '\0'){
                if(counter == DASH_COUNT-1){ // print letter
                    i=0;
                    fprintf(stdout,"%c\n", morseToEnglish(code));
                    // fflush(stdout);
                    code[0] = '\0';
                    code[1] = '\0';
                    code[2] = '\0';
                    code[3] = '\0';
                    code[4] = '\0';
                    code[5] = '\0';
                }
            }else if(counter == SPACE_COUNT-1){ // space
                printf(" ");
            }
        }

        counter++;
        vTaskDelay(pdMS_TO_TICKS(COUNT_TIME));  // Delay for 1 second
    }
}