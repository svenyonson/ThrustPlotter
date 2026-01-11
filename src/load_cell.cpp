// ===== src/load_cell.cpp =====
#include "load_cell.h"
#include "config.h"
#include <HX711.h>

static HX711 scale;
static bool initialized = false;

bool initLoadCell(uint8_t doutPin, uint8_t sckPin) {
    scale.begin(doutPin, sckPin);
    
    if (!scale.is_ready()) {
        Serial.println("HX711 not found. Check wiring.");
        return false;
    }
    
    // Set calibration factor (adjust this during calibration)
    scale.set_scale(LOAD_CELL_CALIBRATION_FACTOR);
    
    // Tare the scale on startup
    Serial.println("Taring load cell...");
    scale.tare();
    
    initialized = true;
    Serial.println("Load cell ready");
    return true;
}

float readThrust() {
    if (!initialized || !scale.is_ready()) {
        return 0.0;
    }
    
    // Read average of 1 sample (you can increase for stability)
    float reading = scale.get_units(1);
    
    // Return absolute value (thrust is always positive)
    return abs(reading);
}

void tareLoadCell() {
    if (!initialized) {
        return;
    }
    
    Serial.println("Taring load cell...");
    scale.tare();
}
/*
void calibrateLoadCell(float knownWeight) {
    if (!initialized || !scale.is_ready()) {
        Serial.println("Cannot calibrate: load cell not ready");
        return;
    }
    
    Serial.println("Place known weight on load cell...");
    delay(2000);
    
    // Read the raw value
    long reading = scale.get_units(10);
    
    // Calculate calibration factor
    float calibrationFactor = reading / knownWeight;
    scale.set_scale(calibrationFactor);
    
    Serial.print("New calibration factor: ");
    Serial.println(calibrationFactor);
    Serial.println("Update LOAD_CELL_CALIBRATION_FACTOR in config.h with this value");
}
*/
bool isLoadCellReady() {
    return initialized && scale.is_ready();
}

void setLoadCellCalibration(float calibrationFactor) {
    if (initialized && calibrationFactor != 0.0) {
        scale.set_scale(calibrationFactor);
        Serial.print("Updated load cell calibration to: ");
        Serial.println(calibrationFactor, 2);
    }
}