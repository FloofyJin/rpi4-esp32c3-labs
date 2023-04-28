#include <stdio.h>

void hid_demo_task(void *pvParameters){
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while(1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        if(sec_conn){
            ESP_LOGI(HID_DEMO_TAG, "send volume");
            send volum_up = true;
            esp_hidd_send_consumer_value(hid_conn_id, "HID_CONSUME_VOLUME_UP, true");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            if(send_volum_up){
                send_volum_up = false;
                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_UP, false);
                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_DOWN, true);
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_DOWN, false);
            }
        }
    }
}

void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
