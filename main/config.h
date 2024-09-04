#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#define MATRIX_ROWS 5
#define MATRIX_COLS 6

// #define DEVICE_ROLE ROLE_PRIMARY
// #define DEVICE_ROLE ROLE_LEFT
#define DEVICE_ROLE ROLE_RIGHT

#define DISPLAY_SUPPORT true
#define DISPLAY_CS_PIN GPIO_NUM_10
#define DISPLAY_MOSI_PIN GPIO_NUM_11
#define DISPLAY_SCLK_PIN GPIO_NUM_12
#define DISPLAY_BUSY_PIN GPIO_NUM_3
#define DISPLAY_RST_PIN GPIO_NUM_13
#define DISPLAY_DC_PIN GPIO_NUM_9
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 296

#endif
