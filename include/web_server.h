#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

// Initialize web server and routes
bool initWebServer();

// Handle web server requests (call in loop)
void handleWebServer();

#endif
