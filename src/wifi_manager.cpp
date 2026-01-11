// ===== src/wifi_manager.cpp =====
#include "wifi_manager.h"
#include "config_page.h"
#include "config.h"
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <HX711.h>

// Configuration
#define AP_SSID "ThrustPlotter-Config"
#define AP_PASSWORD ""  // Open network for easier access
#define DNS_PORT 53
#define WEB_SERVER_PORT 80
#define BUTTON_HOLD_TIME 3000  // 3 seconds to enter config mode

// Static objects
static DNSServer dnsServer;
static Preferences prefs;
static HX711 configScale;  // Load cell for calibration in config mode

// Shared objects 
extern WebServer server;    // This is the only module sharing the WebServer instance - which is is declared in web_server.cpp

// State variables
static bool configMode = false;
static unsigned long buttonPressStart = 0;
static bool buttonWasPressed = false;
static uint8_t buttonPin = 0;
static uint8_t ledPin = 2;

// Calibration state
static bool calibrationInProgress = false;
static int calibrationCurrentStep = 0;
static String calibrationMessage = "";
static float calibrationResult = 0.0;
static float calibrationKnownWeight = 0.0;
static unsigned long calibrationStepStart = 0;

// Forward declarations for internal functions
static void checkConfigButton();
static void handleRoot();
static void handleSave();
static void handleGetCalibration();
static void handleStartCalibration();
static void handleCalibrationStatus();
static void handleCalibrationProcess();

void initWiFiManager(uint8_t btnPin, uint8_t led) {
    buttonPin = btnPin;
    ledPin = led;
    
    // Initialize preferences
    prefs.begin("wifi-config", false);
}

void handleWiFiManager() {
    // Check for button press to enter config mode
    checkConfigButton();
    
    if (configMode) {
        // Handle captive portal
        dnsServer.processNextRequest();
        server.handleClient();
        
        // Handle calibration process if in progress
        handleCalibrationProcess();
        
        // Blink LED in config mode
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            digitalWrite(ledPin, !digitalRead(ledPin));
            lastBlink = millis();
        }
    }
}

bool isInConfigMode() {
    return configMode;
}

bool connectToSavedWiFi() {
    String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    
    if (ssid.length() == 0) {
        Serial.println("No saved WiFi credentials.");
        return false;
    }
    
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    
    //Serial.print("Password: ");
    //Serial.println(password);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    for (int i=0; i < 5; i++) {
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            break;
        }
        WiFi.reconnect();
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        if (!MDNS.begin("thrustplotter")) {
            Serial.println("Error setting up MDNS responder!");
        } else {
            MDNS.addService("http", "tcp", 80); 
            Serial.println("mDNS responder started. Access at http://thrustplotter.local");
        }
        
        // Wait for things to settle
        static bool hasSettled = false;
        if (!hasSettled) {
            delay(1000);
            Serial.flush();
            hasSettled = true;
        }

        // Set Timezone
        static bool timeSet = false;
        if (!timeSet) {
            String savedTZ = prefs.getString("timezone", "GMT0"); 
            configTime(0, 0, "pool.ntp.org", "time.nist.gov"); 
            setenv("TZ", savedTZ.c_str(), 1);
            tzset();
            timeSet = true;
        }

        // Print verification
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char locBuff[64];
            strftime(locBuff, sizeof(locBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
            
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Local Time: %s\n", locBuff);
            Serial.println("--------------------------");
        }
    }

    return (WiFi.status() == WL_CONNECTED);
}

void enterConfigMode() {
    configMode = true;
    
    // Stop any existing WiFi connection
    WiFi.disconnect();
    delay(100);
    
    // Start Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    // Initialize load cell for calibration
    configScale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
    Serial.println("Load cell initialized for calibration");
    
    // Start DNS server for captive portal
    dnsServer.start(DNS_PORT, "*", IP);
    
    // Setup web server routes
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/api/config", HTTP_GET, []() {
        String ssid = prefs.getString("ssid", "");
        String timezone = prefs.getString("timezone", "GMT0");
        float calibrationFactor = prefs.getFloat("cal_factor", 0.0);
        
        String json = "{";
        json += "\"ssid\":\"" + ssid + "\",";
        json += "\"timezone\":\"" + timezone + "\",";
        json += "\"calibrationFactor\":" + String(calibrationFactor, 2);
        json += "}";
        
        server.send(200, "application/json", json);
    });
    server.on("/api/calibration", HTTP_GET, handleGetCalibration);
    server.on("/api/calibration/start", HTTP_POST, handleStartCalibration);
    server.on("/api/calibration/status", HTTP_GET, handleCalibrationStatus);
    server.on("/api/reboot", HTTP_POST, []() {
        server.send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    });
    server.onNotFound(handleRoot);  // Redirect all unknown requests to config page
    
    server.begin();
    Serial.println("Configuration portal started.");
    Serial.println("Connect to WiFi network: " + String(AP_SSID));
}

static void checkConfigButton() {
    bool buttonPressed = (digitalRead(buttonPin) == LOW);
    
    if (buttonPressed && !buttonWasPressed) {
        // Button just pressed
        buttonPressStart = millis();
        buttonWasPressed = true;
    } else if (!buttonPressed && buttonWasPressed) {
        // Button released
        buttonWasPressed = false;
    } else if (buttonPressed && buttonWasPressed) {
        // Button being held
        if (millis() - buttonPressStart >= BUTTON_HOLD_TIME && !configMode) {
            Serial.println("Entering configuration mode...");
            enterConfigMode();
        }
    }
}

static void handleRoot() {
    Serial.println("Serving wifi config page");
    server.send(200, "text/html", CONFIG_PAGE);
}

static void handleSave() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String tz = server.arg("timezone");
    
    Serial.println("Saving WiFi credentials:");
    Serial.println("SSID: " + ssid);
    
    // Save to preferences
    prefs.putString("ssid", ssid);

    // if zero length, then user left blank as "unchanged"
    if (password.length() > 0) {
        prefs.putString("password", password);
    }
    prefs.putString("timezone", tz);
    
    server.send(200, "text/plain", "OK");
    
    delay(2000);
    
    Serial.println("Restarting to connect to new WiFi...");
    ESP.restart();
}

static void handleGetCalibration() {
    float calibrationFactor = prefs.getFloat("cal_factor", 0.0);
    
    String json = "{\"calibrationFactor\":" + String(calibrationFactor, 2) + "}";
    server.send(200, "application/json", json);
}

static void handleStartCalibration() {
    if (calibrationInProgress) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Calibration already in progress\"}");
        return;
    }
    
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing body\"}");
        return;
    }
    
    // Parse JSON body
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        return;
    }
    
    calibrationKnownWeight = doc["knownWeight"].as<float>();
    
    if (calibrationKnownWeight <= 0) {
        server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid weight\"}");
        return;
    }
    
    // Check if load cell is ready
    if (!configScale.is_ready()) {
        server.send(500, "application/json", "{\"success\":false,\"message\":\"Load cell not ready\"}");
        return;
    }
    
    // Start calibration process
    calibrationInProgress = true;
    calibrationCurrentStep = 1;
    calibrationMessage = "Remove all weight...";
    calibrationStepStart = millis();
    calibrationResult = 0.0;
    
    Serial.println("Starting calibration with known weight: " + String(calibrationKnownWeight, 1) + "g");
    
    server.send(200, "application/json", "{\"success\":true}");
}

static void handleCalibrationStatus() {
    String json = "{";
    json += "\"step\":" + String(calibrationCurrentStep) + ",";
    json += "\"message\":\"" + calibrationMessage + "\",";
    json += "\"complete\":" + String(calibrationInProgress ? "false" : "true") + ",";
    json += "\"success\":" + String(calibrationResult > 0 ? "true" : "false");
    
    if (calibrationResult > 0) {
        json += ",\"calibrationFactor\":" + String(calibrationResult, 2);
    }
    
    json += "}";
    
    server.send(200, "application/json", json);
}

static void handleCalibrationProcess() {
    if (!calibrationInProgress) return;
    
    unsigned long elapsed = millis() - calibrationStepStart;
    
    switch (calibrationCurrentStep) {
        case 1:  // Waiting to tare
            if (elapsed < 5000) {
                calibrationMessage = "Step 1/2: Remove all weight... (" + String(5 - (elapsed/1000)) + "s)";
            } else {
                // Tare the scale
                Serial.println("Taring scale...");
                configScale.set_scale();
                configScale.tare();
                calibrationCurrentStep = 2;
                calibrationStepStart = millis();
                calibrationMessage = "Place known weight on scale...";
                Serial.println("Tare complete. Place weight on scale.");
            }
            break;
            
        case 2:  // Waiting to take measurements
            if (elapsed < 5000) {
                calibrationMessage = "Step 2/2: Place " + String(calibrationKnownWeight, 1) + "g weight... (" + String(5 - (elapsed/1000)) + "s)";
            } else {
                calibrationCurrentStep = 3;
                calibrationResult = 0.0;  // Reset for accumulating readings
                calibrationMessage = "Taking measurements (1/5)...";
                calibrationStepStart = millis();
                Serial.println("Starting measurements...");
            }
            break;
            
        case 3:  // Taking 5 measurements
        case 4:
        case 5:
        case 6:
        case 7: {
            int measurementNum = calibrationCurrentStep - 2;
            
            if (elapsed < 1000) {
                calibrationMessage = "Taking measurement " + String(measurementNum) + "/5...";
            } else {
                // Take one measurement
                if (configScale.is_ready()) {
                    long reading = configScale.get_units(10);  // Average of 10 reads
                    calibrationResult += reading;
                    Serial.println("Measurement " + String(measurementNum) + ": " + String(reading));
                    
                    if (calibrationCurrentStep < 7) {
                        calibrationCurrentStep++;
                        calibrationStepStart = millis();
                    } else {
                        // Calculate final calibration factor
                        float averageReading = calibrationResult / 5.0;
                        calibrationResult = averageReading / calibrationKnownWeight;
                        
                        Serial.println("Average reading: " + String(averageReading));
                        Serial.println("Calibration factor: " + String(calibrationResult, 2));
                        
                        // Save to preferences
                        prefs.putFloat("cal_factor", calibrationResult);
                        
                        calibrationMessage = "Calibration complete!";
                        calibrationInProgress = false;
                        
                        Serial.println("Calibration saved to preferences");
                    }
                } else {
                    Serial.println("Warning: Load cell not ready during measurement");
                    calibrationStepStart = millis();  // Retry
                }
            }
            break;
        }
    }
}