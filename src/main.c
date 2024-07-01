#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <string.h>
#include <sys/param.h>

#include "app.h"
#include "camera.h"
#include "dht11_report.h"
#include "take_picture.h"
#include "wifi.h"

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_connection();

    raw_camera_application();

    // xTaskCreate(dht11_report, "HDT11", 20480, 0, 1, NULL);
    xTaskCreate(camera_app_main, "CAMERA", 2048, 0, 1, NULL);
}
