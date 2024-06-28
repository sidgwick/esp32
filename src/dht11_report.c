#include <dht11.h>

#include "driver/gpio.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

//
#include "app.h"
#include "dht11_report.h"
#include "http/client.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048
#define FRAM_HTTP_HOST "farm.iuwei.fun"

static const char *TAG = "DHT11";

static void http_with_url(float temperature, float humidity) {
    char buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};

    esp_http_client_config_t config = {
        .host = FRAM_HTTP_HOST,
        .path = "/prometheus/report",
        .event_handler = http_event_handler,
        .user_data = buffer,  // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    int post_data_len = sprintf(buffer, "{\"temperature\":%.2f,\"humidity\":%.2f}", temperature, humidity);
    esp_http_client_set_url(client, "http://" FRAM_HTTP_HOST "/prometheus/report");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, buffer, post_data_len);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void dht11_report() {
    DHT11_init(GPIO_NUM_4);

    while (1) {
        struct dht11_reading res = DHT11_read();

        ESP_LOGI(TAG, "Temperature is %.2f", res.temperature);
        ESP_LOGI(TAG, "Humidity is %.2f\n", res.humidity);
        ESP_LOGI(TAG, "Status code is %d\n", res.status);

        if (res.status == DHT11_OK) {
            http_with_url(res.temperature, res.humidity);
        }

        vTaskDelay(8000 / portTICK_RATE_MS);
    }
}
