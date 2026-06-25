#include "MicDetector.h"

#include "BoardConfig.h"

#include <driver/i2s.h>
#include <math.h>

namespace R48Mic {

namespace {

Snapshot state;
bool driverInstalled = false;

void stopDriver() {
  if (driverInstalled) {
    i2s_driver_uninstall(I2S_NUM_0);
    driverInstalled = false;
  }
  state.ready = false;
}

}  // namespace

void begin(bool enabled, float threshold) {
  configure(enabled, threshold);
}

void configure(bool enabled, float threshold) {
  state.enabled = enabled;
  state.threshold = constrain(threshold, 100.0f, 12000.0f);
  state.tone = false;
  if (!enabled) {
    stopDriver();
    state.status = "mic disabled";
    return;
  }
  if (driverInstalled) {
    state.ready = true;
    state.status = "mic ready";
    return;
  }

  i2s_config_t config{};
  config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = 16000;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 4;
  config.dma_buf_len = 128;
  config.use_apll = false;
  config.tx_desc_auto_clear = false;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins{};
  pins.bck_io_num = PIN_MIC_SCK;
  pins.ws_io_num = PIN_MIC_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = PIN_MIC_SD;

  if (i2s_driver_install(I2S_NUM_0, &config, 0, nullptr) != ESP_OK ||
      i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK) {
    state.errors++;
    state.status = "mic init failed";
    stopDriver();
    return;
  }

  i2s_zero_dma_buffer(I2S_NUM_0);
  driverInstalled = true;
  state.ready = true;
  state.status = "mic ready";
}

void loop(bool packCurrentActive) {
  if (!state.enabled || !state.ready || millis() - state.lastReadMs < 250) return;

  static int16_t samples[256];
  size_t bytesRead = 0;
  const esp_err_t result = i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytesRead, 0);
  state.lastReadMs = millis();
  if (result != ESP_OK || bytesRead < sizeof(int16_t) * 16) {
    state.errors++;
    state.status = "mic waiting";
    return;
  }

  const size_t count = bytesRead / sizeof(int16_t);
  int64_t sumSquares = 0;
  uint32_t peak = 0;
  for (size_t i = 0; i < count; ++i) {
    const int32_t v = samples[i];
    const uint32_t mag = abs(v);
    if (mag > peak) peak = mag;
    sumSquares += static_cast<int64_t>(v) * v;
  }

  state.rms = static_cast<uint32_t>(sqrtf(sumSquares / static_cast<float>(count)));
  state.peak = peak;
  if (!packCurrentActive && state.rms < state.floor * 2U) {
    state.floor = max<uint32_t>(80, (state.floor * 15U + state.rms) / 16U);
  }

  state.tone = state.rms >= max(state.threshold, state.floor * 2.5f);
  if (state.tone) state.detections++;
  state.status = state.tone ? "work tone likely" : "ambient";
}

bool toneDetected() {
  return state.enabled && state.ready && state.tone;
}

Snapshot snapshot() {
  return state;
}

}  // namespace R48Mic
