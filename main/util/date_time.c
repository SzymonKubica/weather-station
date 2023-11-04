// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
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
void allocate_request_fields(struct Request *request);
void populate_request_fields(struct Request *request);
void free_request_fields(struct Request *request);
void allocate_system_time();

void update_time()
{
    ESP_LOGI(TAG, "Sending HTTP request to get the current time...");
    struct Request *request = calloc(1, sizeof(struct Request));

    allocate_request_fields(request);
    populate_request_fields(request);

    cJSON *json;
    int status = send_http_request(request, &json);

    if (status == REQUEST_FAILED) {
        return;
    }

    free_request_fields(request);
    free(request);

    ESP_LOGD(TAG, "Response received:\n%s\n", cJSON_Print(json));
    ESP_LOGI(TAG, "Extracting time data...");
    extract_time_data(json);
    ESP_LOGI(TAG, "System time updated successfully.");

    free(json);
}

void allocate_system_time()
{
    system_time.date_time = malloc(sizeof(struct tm));
    system_time.date_time_utc = malloc(sizeof(struct tm));
}

char *allocate_str(int length);

void allocate_request_fields(struct Request *request)
{
    request->web_server = allocate_str(strlen(TIME_SERVER));
    request->web_port = allocate_str(strlen(TIME_SERVER_PORT));
    request->web_path = allocate_str(strlen(TIME_SERVER_PATH));
    request->body = allocate_str(strlen(TIME_REQUEST));
    request->max_attempts = 3;
}

void copy_str(char *destination, char *str);
void populate_request_fields(struct Request *request)
{
    copy_str(request->web_server, TIME_SERVER);
    copy_str(request->web_port, TIME_SERVER_PORT);
    copy_str(request->web_path, TIME_SERVER_PATH);
    copy_str(request->body, TIME_REQUEST);
}

void copy_str(char *destination, char *str)
{
    strncpy(destination, str, strlen(str) + 1);
}

void free_request_fields(struct Request *request)
{
    free(request->web_server);
    free(request->web_port);
    free(request->web_path);
    free(request->body);
}

char *allocate_str(int length)
{
    // Need to account for the string terminator character.
    return calloc(length + 1, sizeof(char));
}

char *extract_str_field(cJSON *response, const char *field_name);

void extract_time_data(cJSON *response)
{
    const char *fmt = "%Y-%m-%dT%T%z";
    char *date_time = extract_str_field(response, "datetime");
    char *date_time_utc = extract_str_field(response, "utc_datetime");

    strptime(date_time, fmt, system_time.date_time);
    strptime(date_time_utc, fmt, system_time.date_time_utc);

    char buffer[30];
    strftime(buffer, 30, fmt, system_time.date_time);
    ESP_LOGI(TAG, "Extracted time:\n%s\n", buffer);
}

char *extract_str_field(cJSON *response, const char *field_name)
{
    return cJSON_GetObjectItem(response, field_name)->valuestring;
}
