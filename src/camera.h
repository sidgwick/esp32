#pragma once

// XCLK 过高会导致 `cam_hal: EV-EOF-OVF` 错误: https://github.com/espressif/esp32-camera/issues/535
#define CAM_XCLK_FREQ_HZ 10 * 1000 * 1000

#define CAM_PIN_PWDN -1   // power down is not used
#define CAM_PIN_RESET -1  // software reset will be performed
#define CAM_PIN_XCLK 21
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

void raw_camera_application();