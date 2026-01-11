#include <Arduino.h>
#include <LittleFS.h>
#include "config.h"
#include "wifi_manager.h"
#include "load_cell.h"
#include "run_manager.h"
#include "data_logger.h"
#include "config.h"
#include "web_server.h"
#include <Preferences.h>


// Timing variables
unsigned long lastSample = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Pin setup
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("\n\n=== Thrust Meter Starting ===");
    
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("ERROR: LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS mounted successfully");
    
    // Initialize WiFi
    delay(2000);
    initWiFiManager(BUTTON_PIN, LED_PIN);
    if (connectToSavedWiFi()) {
        Serial.flush();
        delay(1000);
        Serial.println("Connected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("No WiFi. Hold button for config mode.");
        return;
    }

    // Load calibration factor from preferences
    Preferences prefs;
    prefs.begin("wifi-config", true);  // Read-only
    float calibrationFactor = prefs.getFloat("cal_factor", LOAD_CELL_CALIBRATION_FACTOR);
    prefs.end();
    
    if (calibrationFactor != 0.0) {
        Serial.print("Loaded calibration factor: ");
        Serial.println(calibrationFactor);
        // Apply to load cell
        setLoadCellCalibration(calibrationFactor);
    } else {
        Serial.println("Using default calibration factor");
    }


    // Initialize modules
    if (!initLoadCell(HX711_DOUT_PIN, HX711_SCK_PIN)) {
        Serial.println("WARNING: Load cell initialization failed");
    } else {
        Serial.println("Load cell initialized");
    }
    
    if (!initRunManager()) {
        Serial.println("ERROR: Run manager initialization failed");
    } else {
        Serial.println("Run manager initialized");
    }
    
    if (!initDataLogger()) {
        Serial.println("ERROR: Data logger initialization failed");
    } else {
        Serial.println("Data logger initialized");
    }
        
    if (!initWebServer()) {
        Serial.println("ERROR: Web server initialization failed");
    } else {
        Serial.println("Web server started");
    }
    
    Serial.println("=== Initialization Complete ===\n");
}

int read_test = 0;
void loop() {

    if (read_test) {
        float thrust = readThrust();
        Serial.print("one reading:\t");
        Serial.println(thrust, 2);
         delay(10);
    }
    else {
        // Handle WiFi configuration
        handleWiFiManager();
        
        if (!isInConfigMode()) {
            // Handle web server requests
            handleWebServer();
            
            // If a run is active, log samples at the configured rate
            if (isRunActive()) {
                unsigned long currentTime = millis();
                if (currentTime - lastSample >= SAMPLE_RATE_MS) {
                    float thrust = readThrust();
                    unsigned long timestamp = currentTime - getCurrentRun().startTime;

                    if (getSampleCount() == 0 && thrust <= 0.5) {
                        // Beginning of run. Discard zero/noise sample and advance start time (start all runs with non-zero thrust)
                        resetStartTime(currentTime);
                        lastSample = currentTime;
                    }
                    else {
                        if (logSample(thrust, timestamp)) {
                            // Sample logged successfully
                            lastSample = currentTime;
                        } else {
                            Serial.println("ERROR: Failed to log sample");
                        }
                    }
                }
            }
            // LED indicator
            if (WiFi.status() == WL_CONNECTED) {
                digitalWrite(LED_PIN, isRunActive() ? (millis() % 500 < 250) : HIGH);
            } else {
                digitalWrite(LED_PIN, LOW);
            }
        }
        else {

        }
    }
}
