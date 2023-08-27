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

#include "http_client.h"
#include "util.h"

#define SOCKET_READ_SUCCESS 0

enum SocketConnectionStatus {
    SOCKET_CONNECTION_SUCCESS,
    SOCKET_ALLOCATION_FAILED,
    SOCKET_CONNECTION_FAILED,
};

const char *socket_status_message[] = {
    [SOCKET_CONNECTION_SUCCESS] = "... Socket connection successful!",
    [SOCKET_ALLOCATION_FAILED] = "... Failed to allocate socket.",
    [SOCKET_CONNECTION_FAILED] = "Failed to perform socket connection."};

static const char *TAG = "HTTP_CLIENT";

void extract_json_from_response(char *response, char *json_string,
                                int response_length);

int connect_socket(int *socket, struct addrinfo *res);
int read_response(int *socket);

void http_request_task(void *pvParameters)
{
    const struct Request *request = (struct Request *)pvParameters;

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s;

    int attempts = 0;
    while (attempts < request->max_attempts) {
        int err =
            getaddrinfo(request->web_server, request->web_port, &hints, &res);

        if (err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p.", err, res);
            wait_seconds(1);
            attempts++;
            continue;
        }

        /* Code to print the resolved IP.
           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real"
           code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        /* Perform socket connection */
        s = socket(res->ai_family, res->ai_socktype, 0);
        enum SocketConnectionStatus status = connect_socket(&s, res);
        if (status != SOCKET_CONNECTION_SUCCESS) {
            ESP_LOGE(TAG, "%s", socket_status_message[status]);
            wait_seconds(4);
            attempts++;
            continue;
        }
        ESP_LOGI(TAG, "%s", socket_status_message[status]);

        /* Send the HTTP request over the socket */
        if (write(s, request->body, strlen(request->body)) < 0) {
            ESP_LOGE(TAG, "... Socket send failed.");
            close(s);
            wait_seconds(4);
            attempts++;
            continue;
        }
        ESP_LOGI(TAG, "... Socket send succeeded.");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                       sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... Failed to set socket receiving timeout.");
            close(s);
            wait_seconds(4);
            attempts++;
            continue;
        }
        ESP_LOGI(TAG, "... Socket receiving timeout set successfully.");

        int read_status = read_response(&s);
        if (read_status != SOCKET_READ_SUCCESS) {
            ESP_LOGE(TAG, "... Unable to read from the socket.");
            wait_seconds(4);
            attempts++;
            continue;
        }
    }
}

int connect_socket(int *socket, struct addrinfo *res)
{
    if (*socket < 0) {
        freeaddrinfo(res);
        return SOCKET_ALLOCATION_FAILED;
    }
    ESP_LOGI(TAG, "... allocated socket");

    if (connect(*socket, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
        close(*socket);
        freeaddrinfo(res);
        return SOCKET_CONNECTION_FAILED;
    }

    ESP_LOGI(TAG, "... connected");
    freeaddrinfo(res);
    return SOCKET_CONNECTION_SUCCESS;
}

int read_response(int *socket)
{
    char *response = calloc(10000, sizeof(char));
    char *response_ptr = response;
    int r;
    char recv_buf[64];

    do {
        bzero(recv_buf, sizeof(recv_buf));
        r = read(*socket, recv_buf, sizeof(recv_buf) - 1);
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
    ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.",
             r, errno);
    close(*socket);
    free(json_string);
    return SOCKET_READ_SUCCESS;
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
