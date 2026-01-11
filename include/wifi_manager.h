#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// Initialize the WiFi manager
void initWiFiManager(uint8_t buttonPin, uint8_t ledPin);

// Main handler - call this in loop()
void handleWiFiManager();

// Connect to saved WiFi credentials
bool connectToSavedWiFi();

// Check if currently in config mode
bool isInConfigMode();

// Force enter config mode (optional, for testing)
void enterConfigMode();

#endif
