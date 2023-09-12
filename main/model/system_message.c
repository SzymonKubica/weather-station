// Copyright (c) 2023 Szymon Kubica
// SPDX-License-Identifier: MIT
#include "system_message.h"

QueueHandle_t ir_remote_msg_queue = NULL;
QueueHandle_t system_msg_queue = NULL;
QueueHandle_t display_msg_queue = NULL;
QueueHandle_t forecast_msg_queue = NULL;

struct IRRemoteMessage ir_remote_message;
struct DisplayMessage display_message;
struct SystemMessage system_message;
struct ForecastRequest forecast_request_message;
