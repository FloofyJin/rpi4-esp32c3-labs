#include <stdio.h>
#include "esp_log.h"
#include <inttypes.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "i2c_master_init.h"

#define I2C_MASTER_SCL_IO 8        
#define I2C_MASTER_SDA_IO 10        
#define I2C_MASTER_NUM    I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 1000000

void i2c_master_init(){
        i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master ={.clk_speed = I2C_MASTER_FREQ_HZ,},
    };

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}