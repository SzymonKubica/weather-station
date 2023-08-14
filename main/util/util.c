#include "util.h"

void wait_seconds(int seconds) {
  vTaskDelay(seconds * 1000 / portTICK_PERIOD_MS);
}

void initialize_non_volatile_flash(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}
