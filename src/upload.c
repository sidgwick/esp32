/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "upload.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>  // struct addrinfo
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

static const char *TAG = "upload-picture";

size_t read_length_prefixed_content(int sock, char *rx_buffer) {
    // READ prefix
    size_t len = recv(sock, rx_buffer, 4, 0);
    if (len <= 0) {
        ESP_LOGE(TAG, "prefix length recv failed: errno %s", strerror(errno));
        return -2;
    }

    int wl = ntohl(*((size_t *)rx_buffer));
    ESP_LOGI(TAG, "read length=%d from remote", wl);
    len = recv(sock, rx_buffer, wl, 0);
    if (len != wl) {
        ESP_LOGE(TAG, "content recv failed: errno %s", strerror(errno));
        return -2;
    }

    return len;
}

int send_image(int sock, const char *payload, size_t len) {
    char rx_buffer[128];

    ESP_LOGI(TAG, "Sending image to remote, sock=%d", sock);
    int wl = htonl(len);
    send(sock, (char *)(&wl), 4, 0);

    int err = send(sock, payload, len, 0);
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %s", strerror(errno));
        return -1;
    }

    ESP_LOGI(TAG, "Sending image to remote done, waitting for response now");
    len = read_length_prefixed_content(sock, rx_buffer);
    if (len <= 0) {
        ESP_LOGE(TAG, "Error occurred during receiving: errno %s", strerror(errno));
        return -1;
    }

    // Null-terminate whatever we received and treat like a string
    rx_buffer[len] = 0;
    ESP_LOGI(TAG, "Image %s processed by remote", rx_buffer);

    return 0;
}

void upload_picture(const char *payload, size_t len) {
    static int sock;

    struct sockaddr_in dest_addr;

    if (sock <= 0) {
        char host_ip[] = HOST_IP_ADDR;
        int addr_family = 0;
        int ip_protocol = 0;

        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        ESP_LOGI(TAG, "Socket created, sock=%d, connecting to %s:%d", sock, host_ip, PORT);

        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            return;
        }

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %s", strerror(errno));
            return;
        }

        ESP_LOGI(TAG, "Successfully connected");
    }

    int rsp = send_image(sock, payload, len);
    if (rsp != 0) {
        ESP_LOGI(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
        sock = -1;
    }
}
