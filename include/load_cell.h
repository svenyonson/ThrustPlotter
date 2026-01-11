#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include <Arduino.h>

// Initialize the load cell
bool initLoadCell(uint8_t doutPin, uint8_t sckPin);

// Read current thrust in grams
float readThrust();

// Tare the load cell (zero it)
void tareLoadCell();

// Calibrate with known weight
//void calibrateLoadCell(float knownWeight);
void setLoadCellCalibration(float calibrationFactor);

// Check if load cell is ready
bool isLoadCellReady();

#endif
