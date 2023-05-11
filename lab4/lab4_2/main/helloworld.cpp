#include <stdio.h>
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

DFRobot_LCD lcd(16,2);

void app_main(void)
{
    lcd.init();
    while(1){
        lcd.setRGB(0,255,0);
        lcd.printstr("Hello CSE121!", lcd, 0);
        lcd.printstr("Jinsung Park", lcd, 1);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}
