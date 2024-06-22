#pragma once
#include <strings.h>

#define HOST_IP_ADDR "192.168.0.104"
#define PORT 8458

void upload_picture(const char *payload, size_t len);