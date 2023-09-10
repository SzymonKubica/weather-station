#include "date_time.h"
#include "../libs/http-client/http_client.h"
#include "esp_log.h"
#include "system_message.h"
#include <stdlib.h>
#include <string.h>

#define SERVER "worldtimeapi.org"
#define PORT "80"
#define PATH "http://worldtimeapi.org/api/timezone/Europe/London"

#define TAG "DATE_TIME"

struct DateTime system_time = {};

char *TIME_SERVER = SERVER;
char *TIME_SERVER_PORT = PORT;
char *TIME_SERVER_PATH = PATH;

char *TIME_REQUEST = "GET " PATH " HTTP/1.0\r\n"
                     "Host: " SERVER " \r\n"
                     "User-Agent: esp-idf/1.0 esp32\r\n"
                     "\r\n";

void extract_time_data(cJSON *response);

void allocate_system_time()
{
    system_time.date_time = malloc(sizeof(struct tm));
    system_time.date_time_utc = malloc(sizeof(struct tm));
}

void update_time()
{
    ESP_LOGI(TAG, "Sending HTTP request to get the current time...");
    struct Request *request = calloc(1, sizeof(struct Request));
    request->web_server = calloc(strlen(TIME_SERVER) + 1, sizeof(char));
    request->web_port = calloc(strlen(TIME_SERVER_PORT) + 1, sizeof(char));
    request->web_path = calloc(strlen(TIME_SERVER_PATH) + 1, sizeof(char));
    request->body = calloc(strlen(TIME_REQUEST) + 1, sizeof(char));
    request->max_attempts = 3;

    strncpy(request->web_server, TIME_SERVER, strlen(TIME_SERVER) + 1);
    strncpy(request->web_port, TIME_SERVER_PORT, strlen(TIME_SERVER_PORT) + 1);
    strncpy(request->web_path, TIME_SERVER_PATH, strlen(TIME_SERVER_PATH) + 1);
    strncpy(request->body, TIME_REQUEST, strlen(TIME_REQUEST) + 1);
    cJSON *json;
    int status = send_http_request(request, &json);

    if (status == REQUEST_FAILED) {
        return;
    }

    free(request->web_server);
    free(request->web_port);
    free(request->web_path);
    free(request->body);
    free(request);

    printf("%s\n", cJSON_Print(json));
    ESP_LOGI(TAG, "Extracting time data...");
    extract_time_data(json);
    free(json);
    ESP_LOGI(TAG, "System time updated successfully.");
}

void extract_time_data(cJSON *response)
{
    char *date_time = cJSON_GetObjectItem(response, "datetime")->valuestring;
    char *date_time_utc =
        cJSON_GetObjectItem(response, "utc_datetime")->valuestring;

    strptime(date_time, "%Y-%m-%dT%T%z", system_time.date_time);
    strptime(date_time_utc, "%Y-%m-%dT%T%z", system_time.date_time_utc);

    char buffer[30];
    strftime(buffer, 30, "%Y-%m-%dT%T%z", system_time.date_time);
    printf("%s\n", buffer);
}
