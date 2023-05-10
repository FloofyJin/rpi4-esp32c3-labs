#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
// #include "DFRobot_LCD.h"
// #include "i2c_scanner.h"

#define I2C_MASTER_SCL_IO 8        
#define I2C_MASTER_SDA_IO 10        
#define I2C_MASTER_NUM    I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000  

// #define LCD_ADDR 0x7C

static const char *TAG = "lab4";

// DFRobot_LCD lcd(0x27, 16, 2); // Initialize the LCD module

// extern "C" { 
    // void i2c_master_init(void);
    // void i2c_write(uint8_t reg, uint8_t data);
    // uint8_t i2c_read(uint8_t reg);
//     void app_main();
// }

static esp_err_t i2c_master_init(void){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SDA_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to i2c_param_config %d", err);
        return err;
    }

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to i2c_driver_install %d", err);
        return err;
    }
    return err;
}

// void i2c_write(uint8_t reg, uint8_t data){
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, reg, true);
//     i2c_master_write_byte(cmd, data, true);
//     i2c_master_stop(cmd);
//     i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
//     i2c_cmd_link_delete(cmd);
// }

static void lcd_init(void){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // display on
    i2c_master_write_byte(cmd, 0x7C | I2C_MASTER_WRITE, true);//slave

    i2c_master_write_byte(cmd, 0x80, true);//control
    i2c_master_write_byte(cmd, 0x38, true);//data
    // clear display
    i2c_master_write_byte(cmd, 0x80, true);//control
    i2c_master_write_byte(cmd, 0x0C, true);//data
    // function set
    i2c_master_write_byte(cmd, 0x80, true);//control
    i2c_master_write_byte(cmd, 0x01, true);//data
    
    i2c_master_write_byte(cmd, 0x00, true);//control
    i2c_master_write_byte(cmd, 0x06, true);//data

    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to 1st write %d", err);
    }
    i2c_cmd_link_delete(cmd);
}

static void write_hello(){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // set DDRAM
    i2c_master_write_byte(cmd, (0x7C<<1) | I2C_MASTER_WRITE, true);//slave
    i2c_master_write_byte(cmd, 0x80, true);//control
    i2c_master_write_byte(cmd, 0x80, true);//data
    // write data to RAM
    i2c_master_write_byte(cmd, 0x40, true);
    i2c_master_write_byte(cmd, 0x48, true);

    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Failed to write hello %d", err);
    }
    i2c_cmd_link_delete(cmd);
}

void app_main(void)
{
    esp_err_t err = i2c_master_init();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master");
        return;
    }

    lcd_init();
    // write_hello();

    // while(true){
    //     lcd.init();

    //     lcd.setRGB(0, 255, 0);
    //     lcd.printstr("Hello CSE121!");
    //     lcd.setCursor(0, 1);
    //     lcd.printstr("Jinsung");
    // }

}
