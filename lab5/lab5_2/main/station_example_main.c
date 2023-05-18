/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "driver/i2c.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
//////// above
// wifi defines
////////

static const char *TAG = "wifi station";

// char *location = "";

float esp_temp;
float esp_hum;

//////// start
// humidity and temperature sensor
////////
#define I2C_MASTER_SCL_IO 8 /*!< GPIO number for I2C master SCL */
#define I2C_MASTER_SDA_IO 10 /*!< GPIO number for I2C master SDA */
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_FREQ_HZ 100000
#define SHTC3_SENSOR_ADDR 0x70 /*!< SHTC3 12C address */
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

static float calculate_humidity(uint16_t raw_humidity)
{
    return 100.0 * (float)raw_humidity / 65535.0;
}

static float calculate_temperature(uint16_t raw_temperature)
{
    return raw_temperature * 175.0 / 65535.0 - 45.0;
}

void shtc3_task(){
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
            esp_temp = temperature;
            esp_hum = humidity;
            // sprintf(esp_temp, "%s%.0fC", "Temp: ", temperature);
            // sprintf(esp_hum, "%s%.0f%%", "Hum : ", humidity);
            // ESP_LOGI(TAG, "Humidity: %.2f %%", humidity);
            // ESP_LOGI(TAG, "Temperature: %.2f C (%.2f F)", temperature, (temperature*1.8)+32);
        } else {
            ESP_LOGI(TAG, "Failed to read data from SHTC3 sensor %d", err);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

////////
// humidity and temperature sensor
//////// end

//////// start
// http
////////
char *rpi_server;
// #define RPI_SERVER = "example.com"
#define RPI_PORT "1234"
#define RPI_PATH "/"

// static char *RPI_REQUEST = "GET " RPI_PATH " HTTP/1.0\r\n"
//     "Host: "RPI_SERVER":"RPI_PORT"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "Accept: */*\r\n"
//     "\r\n";
// static char *RPI_REQUEST_a = "GET " RPI_PATH " HTTP/1.0\r\n"
//     "Host: ";
// static char *RPI_REQUEST_b = ":"RPI_PORT"\r\n"
//     "User-Agent: esp-idf/1.0 esp32\r\n"
//     "Accept: */*\r\n"
//     "\r\n";

// static void http_get_task(void *pvParameters)
// {
//     char *RPI_REQUEST = (char *) malloc(1+strlen(RPI_REQUEST_a)+strlen(rpi_server)+strlen(RPI_REQUEST_b));
//     strcpy(RPI_REQUEST, RPI_REQUEST_a);
//     strcat(RPI_REQUEST, rpi_server);
//     strcat(RPI_REQUEST, RPI_REQUEST_b);
//     // printf(RPI_REQUEST);

//     const struct addrinfo hints = {
//         .ai_family = AF_INET,
//         .ai_socktype = SOCK_STREAM,
//     };
//     struct addrinfo *res;
//     struct in_addr *addr;
//     int s, r;
//     char recv_buf[64];

//     while(1) {
//         int err = getaddrinfo(rpi_server, RPI_PORT, &hints, &res);

//         if(err != 0 || res == NULL) {
//             ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         /* Code to print the resolved IP.

//            Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
//         addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
//         ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

//         s = socket(res->ai_family, res->ai_socktype, 0);
//         if(s < 0) {
//             ESP_LOGE(TAG, "... Failed to allocate socket.");
//             freeaddrinfo(res);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//             continue;
//         }
//         ESP_LOGI(TAG, "... allocated socket");

//         if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
//             ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
//             close(s);
//             freeaddrinfo(res);
//             vTaskDelay(4000 / portTICK_PERIOD_MS);
//             continue;
//         }

//         ESP_LOGI(TAG, "... connected");
//         freeaddrinfo(res);

//         if (write(s, RPI_REQUEST, strlen(RPI_REQUEST)) < 0) {
//             ESP_LOGE(TAG, "... socket send failed");
//             close(s);
//             vTaskDelay(4000 / portTICK_PERIOD_MS);
//             continue;
//         }
//         ESP_LOGI(TAG, "... socket send success");

//         struct timeval receiving_timeout;
//         receiving_timeout.tv_sec = 5;
//         receiving_timeout.tv_usec = 0;
//         if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
//                 sizeof(receiving_timeout)) < 0) {
//             ESP_LOGE(TAG, "... failed to set socket receiving timeout");
//             close(s);
//             vTaskDelay(4000 / portTICK_PERIOD_MS);
//             continue;
//         }
//         ESP_LOGI(TAG, "... set socket receiving timeout success");
        

//         /* Read HTTP response */
//         char received_data[128] = "";
//         do {
//             bzero(recv_buf, sizeof(recv_buf));
//             r = read(s, recv_buf, sizeof(recv_buf)-1);
//             // for(int i = 0; i < r; i++) {
//                 // putchar(recv_buf[i]);
//                 // location[loc_i++] = recv_buf[i];
//             // }
//             if(r>0){
//                 strncat(received_data, recv_buf, r);
//             }
//         } while(r > 0);

//         location = strstr(received_data, "\r\n\r\n");
//         if(location != NULL){
//             location+= 4;
//         }
//         fprintf(stdout, "location: %s\n", location);

//         ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
//         close(s);

//         for(int countdown = 10; countdown >= 0; countdown--) {
//             // ESP_LOGI(TAG, "%d... ", countdown);
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//         ESP_LOGI(TAG, "Starting again!");
//     }

//     free(RPI_REQUEST);
// }

char *RPI_POST_REQUEST = "POST " RPI_PATH " HTTP/1.0\r\n"
    "%s:"RPI_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Accept: */*\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s"
    ;
char *POST_DATA = "esp temp: %0.2fC, esp hum: %0.2f%%";

static void http_post_task(void *pvParameters)
{

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while(1) {
        
        char post_data[636];
        sprintf(post_data, POST_DATA, esp_temp, esp_hum);
        // printf("%s", post_data);
        char rpi_post_request[1024];
        sprintf(rpi_post_request, RPI_POST_REQUEST, rpi_server, strlen(post_data), post_data);
        printf("%s",rpi_post_request);

        int err = getaddrinfo(rpi_server, RPI_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, rpi_post_request, strlen(rpi_post_request)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            // for(int i = 0; i < r; i++) {
            //     putchar(recv_buf[i]);
            // }
        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        for(int countdown = 1; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
    // free(RPI_POST_REQUEST);
}
////////
// http
//////// end

//////// start
// wifi
////////
#include "esp_netif.h"

void get_gateway_ip()
{
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif == NULL) {
        // Handle error: Network interface not found
        return;
    }

    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        // Handle error: Failed to get IP info
        return;
    }

    esp_ip4_addr_t gw_ip = ip_info.gw;

    char gw_ip_str[IP4ADDR_STRLEN_MAX];
    esp_ip4addr_ntoa(&gw_ip, gw_ip_str, IP4ADDR_STRLEN_MAX);

    printf("using gateway IP: %s\n", gw_ip_str);
    // strncpy(gw, gw_ip_str,IP4ADDR_STRLEN_MAX);
    rpi_server = malloc(strlen(gw_ip_str)+1);
    // strcpy(rpi_server, gw_ip_str);//when using eth hotspot
    strcpy(rpi_server, "192.168.0.48");//connected to wifi. set ip address manually to the server ip address
}

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{   
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    get_gateway_ip();
}
////////
// wifi
//////// end

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    //Initialize i2c for h and t
    esp_err_t err = i2c_master_init();
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master");
        return;
    }
    //get location from server
    // xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    // http_get_task();

    // shtc3_task(); get temp/hum from sensor
    xTaskCreate(&shtc3_task, "shtc3_task", 4096, NULL, 5, NULL);

    //send post request to server
    xTaskCreate(&http_post_task, "http_post_task", 4096, NULL, 5, NULL);
}

// curl "www.wttr.in/Santa+Cruz?format=%l:+%c+%t"
// curl POST server.py: 
// curl -v -X POST -H "Content-Type: text/plain" -d 'Hello, world!' http://localhost:8000
// curl GET server.py:
// curl -X GET http://localhost:8000