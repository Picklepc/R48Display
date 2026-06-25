#pragma once

#include <Arduino.h>

static constexpr const char *PROJECT_NAME = "R48Display";
static constexpr const char *FIRMWARE_VERSION = "0.2.2";

// Waveshare ESP32-S3-Touch-LCD-1.85 display pinout.
static constexpr int PIN_LCD_BL = 5;
static constexpr int PIN_LCD_CS = 21;
static constexpr int PIN_LCD_SCK = 40;
static constexpr int PIN_LCD_D0 = 46;
static constexpr int PIN_LCD_D1 = 45;
static constexpr int PIN_LCD_D2 = 42;
static constexpr int PIN_LCD_D3 = 41;

// Shared internal I2C bus on variants that include the IO expander.
static constexpr int PIN_I2C_SDA = 11;
static constexpr int PIN_I2C_SCL = 10;
static constexpr uint8_t TCA9554_ADDR = 0x20;

// CST816 touchscreen bus.
static constexpr int PIN_TOUCH_SDA = 1;
static constexpr int PIN_TOUCH_SCL = 3;
static constexpr uint8_t CST816_ADDR = 0x15;
static constexpr int PIN_TOUCH_INT = 4;

// Board buttons and optional onboard LiPo sense. These are safe to read on
// 1.85-inch variants that omit the battery circuit.
static constexpr int PIN_BOOT_BUTTON = 0;
static constexpr int PIN_BATTERY_HOLD = 6;
static constexpr int PIN_BATTERY_KEY = 7;
static constexpr int PIN_BATTERY_ADC = 8;
static constexpr float BATTERY_ADC_RATIO = 3.0f;

// Onboard I2S microphone on the 1.85-inch board variant that includes it.
// Safe to leave disabled on variants without the microphone.
static constexpr int PIN_MIC_WS = 2;
static constexpr int PIN_MIC_SCK = 15;
static constexpr int PIN_MIC_SD = 39;

static constexpr uint16_t DISPLAY_WIDTH = 360;
static constexpr uint16_t DISPLAY_HEIGHT = 360;
