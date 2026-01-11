#ifndef CONFIG_H
#define CONFIG_H

// Hardware pins
#define BUTTON_PIN 0
#define LED_PIN 2
#define HX711_DOUT_PIN 16
#define HX711_SCK_PIN 4

// Filesystem
#define DATA_DIR "/data"
#define RUNS_DIR "/data/runs"
#define CHARTS_DIR "/data/charts"
#define CONFIGS_DIR "/data/configs"

// Load cell configuration
#define LOAD_CELL_CALIBRATION_FACTOR 661.41  // Adjust during calibration
#define SAMPLE_RATE_MS 100  // 10 samples per second

// Web server
#define WEB_SERVER_PORT 80

#endif
