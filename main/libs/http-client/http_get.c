/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "json/cJSON.h"
#include <string.h>

#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "sdkconfig.h"

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.open-meteo.com"
#define WEB_PORT "80"
#define WEB_PATH                                                               \
    "/v1/"                                                                     \
    "forecast?latitude=51.5085&longitude=-0.1257&hourly=temperature_2m,"       \
    "relativehumidity_2m,apparent_temperature&daily=sunrise,sunset,"           \
    "precipitation_probability_max&timezone=Europe%2FLondon&forecast_days=3&"  \
    "models=best_match"

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER " \r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";

void extract_json_from_response(char *response, char *json_string,
                                int response_length);

void http_get_task(void *pvParameters)
{

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while (1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if (err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real"
           code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
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

        char *response = calloc(10000, sizeof(char));
        char *response_ptr = response;

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            for (int i = 0; i < r; i++) {
                *response_ptr = recv_buf[i];
                response_ptr++;
            }
        } while (r > 0);

        int response_length = response_ptr - response;
        for (int i = 0; i < response_length; i++) {
            putchar(*(response + i));
        }
        printf("\n");

        char *json_string = calloc(10000, sizeof(char));
        extract_json_from_response(response, json_string, response_length);
        free(response);

        cJSON *json = cJSON_Parse(json_string);
        printf("%s", json->child->string);
        ESP_LOGI(TAG,
                 "... done reading from socket. Last read return=%d errno=%d.",
                 r, errno);
        close(s);
        for (int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        free(json_string);
        ESP_LOGI(TAG, "Starting again!");
    }
}

void extract_json_from_response(char *response, char *json_string,
                                int response_length)
{
    char *response_ptr = response;
    while (*response_ptr != '{') {
        response_ptr++;
    }
    int traversed_distance = response_ptr - response;
    for (int i = 0; i < response_length - traversed_distance; i++) {
        *json_string = *response_ptr;
        json_string++;
        response_ptr++;
    }
}
