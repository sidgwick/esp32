#include <dht11.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "app.h"
#include "dht11_report.h"

void dht11_report() {
    DHT11_init(GPIO_NUM_4);

    while(1) {
        struct dht11_reading res = DHT11_read();
        printf("Temperature is %d \n", res.temperature);
        printf("Humidity is %d\n", res.humidity);
        printf("Status code is %d\n", res.status);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}
