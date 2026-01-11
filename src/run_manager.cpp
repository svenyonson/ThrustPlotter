// src/run_manager.cpp
#include "run_manager.h"
#include "config.h"
#include "data_logger.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

static RunConfig currentRun;
static bool runActive = false;

// Helper function to sanitize filenames
String sanitizeFilename(const String& input) {
    String output = input;
    output.replace(" ", "_");
    output.replace("/", "-");
    output.replace("\\", "-");
    //output.replace(":", "-");
    output.replace("*", "-");
    output.replace("?", "-");
    output.replace("\"", "");
    output.replace("'", "");
    output.replace("<", "");
    output.replace(">", "");
    output.replace("|", "-");
    output.replace("&", "and");
    output.replace("%", "pct");
    output.replace("#", "num");
    return output;
}

// Helper function to generate timestamp string
String getTimestamp() {
    //time_t now = time(nullptr);
    //struct tm* timeinfo = localtime(&now);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%y-%m-%d_%T", &timeinfo);
        return String(buffer);
    }
 
    return String("badTimestamp");
}

bool initRunManager() {
    // Create config directory if it doesn't exist
    if (!LittleFS.exists(CONFIGS_DIR)) {
        if (!LittleFS.mkdir(CONFIGS_DIR)) {
            Serial.println("Failed to create configs directory");
            return false;
        }
    }
    
    // Initialize current run
    currentRun.name = "";
    currentRun.notes = "";
    currentRun.isActive = false;
    currentRun.startTime = 0;
    currentRun.currentFileName = "";
    
    Serial.println("Run manager initialized");
    return true;
}

bool createRunConfig(const String& name, const String& notes) {
    if (name.length() == 0) {
        Serial.println("Run name cannot be empty");
        return false;
    }
    
    // Sanitize the name for the config filename
    String sanitizedName = sanitizeFilename(name);
    
    // Create JSON document
    JsonDocument doc;
    doc["name"] = name;  // Store original name with spaces
    doc["notes"] = notes;
    doc["created"] = getTimestamp();
    
    // Save to file using sanitized name
    String configPath = String(CONFIGS_DIR) + "/" + sanitizedName + ".json";
    File file = LittleFS.open(configPath, "w");
    
    if (!file) {
        Serial.println("Failed to create config file: " + configPath);
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.println("Created run config: " + name + " (file: " + sanitizedName + ".json)");
    return true;
}

bool startRun(const String& runName) {
    if (runActive) {
        Serial.println("A run is already active. Stop it first.");
        return false;
    }
    
    // Sanitize the name to find the config file
    String sanitizedName = sanitizeFilename(runName);
    String configPath = String(CONFIGS_DIR) + "/" + sanitizedName + ".json";
    
    if (!LittleFS.exists(configPath)) {
        Serial.println("Run config not found: " + runName + " (looked for: " + configPath + ")");
        return false;
    }
    
    File file = LittleFS.open(configPath, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }
    
    // Generate filename with timestamp (sanitized)
    String timestamp = getTimestamp();
    String fileName = sanitizedName + ": " + timestamp + ".csv";
    
    // Create data file
    if (!createDataFile(fileName)) {
        Serial.println("Failed to create data file");
        return false;
    }
    
    // Set current run state (store original name with spaces)
    currentRun.name = doc["name"].as<String>();
    currentRun.notes = doc["notes"].as<String>();
    currentRun.isActive = true;
    currentRun.startTime = millis();
    currentRun.currentFileName = fileName;
    runActive = true;
    
    Serial.println("Started run: " + currentRun.name + " -> " + fileName);
    return true;
}

bool stopRun() {
    if (!runActive) {
        Serial.println("No active run to stop");
        return false;
    }
    
    // Close the data file
    closeDataFile();
    
    Serial.println("Stopped run: " + currentRun.name);
    Serial.println("Data saved to: " + currentRun.currentFileName);
    
    // Reset state
    currentRun.isActive = false;
    runActive = false;
    
    return true;
}

bool deleteRun(const String& runName) {
    // Stop run if it's currently active
    if (runActive && currentRun.name == runName) {
        stopRun();
    }
    
    // Sanitize the name for file operations
    String sanitizedName = sanitizeFilename(runName);
    
    // Delete config file
    String configPath = String(CONFIGS_DIR) + "/" + sanitizedName + ".json";
    if (LittleFS.exists(configPath)) {
        LittleFS.remove(configPath);
        Serial.println("Deleted config: " + configPath);
    }
    
    // Delete all data files for this run (use sanitized name prefix)
    File dir = LittleFS.open(RUNS_DIR);
    if (dir && dir.isDirectory()) {
        File file = dir.openNextFile();
        
        while (file) {
            String fileName = String(file.name());
            // Match files that start with sanitized name
            if (fileName.startsWith(sanitizedName + ": ") && fileName.endsWith(".csv")) {
                String fullPath = String(RUNS_DIR) + "/" + fileName;
                LittleFS.remove(fullPath);
                Serial.println("Deleted data file: " + fileName);
            }
            file = dir.openNextFile();
        }
    }
    
    Serial.println("Deleted run: " + runName);
    return true;
}

RunConfig getCurrentRun() {
    return currentRun;
}

bool isRunActive() {
    return runActive;
}

String getRunConfigsList() {
    JsonDocument doc;
    JsonArray runs = doc.to<JsonArray>();
    
    File dir = LittleFS.open(CONFIGS_DIR);
    if (!dir || !dir.isDirectory()) {
        return "[]";
    }
    
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).endsWith(".json")) {
            JsonDocument configDoc;
            DeserializationError error = deserializeJson(configDoc, file);
            
            if (!error) {
                JsonObject run = runs.add<JsonObject>();
                run["name"] = configDoc["name"];  // Return original name with spaces
                run["notes"] = configDoc["notes"];
                run["created"] = configDoc["created"];
            }
        }
        file = dir.openNextFile();
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

String getRunDataFiles(const String& runName) {
    JsonDocument doc;
    JsonArray files = doc.to<JsonArray>();
    
    // Sanitize the name to match filenames
    String sanitizedName = sanitizeFilename(runName);
    
    File dir = LittleFS.open(RUNS_DIR);
    if (!dir || !dir.isDirectory()) {
        return "[]";
    }
    
    File file = dir.openNextFile();
    while (file) {
        String fileName = String(file.name());
        if (!file.isDirectory() && 
            fileName.startsWith(sanitizedName + ": ") && 
            fileName.endsWith(".csv")) {
            
            JsonObject fileObj = files.add<JsonObject>();
            fileObj["name"] = fileName;
            fileObj["size"] = file.size();
        }
        file = dir.openNextFile();
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

bool updateRunNotes(const String& runName, const String& notes) {
    // Sanitize the name to find the config file
    String sanitizedName = sanitizeFilename(runName);
    String configPath = String(CONFIGS_DIR) + "/" + sanitizedName + ".json";
    
    if (!LittleFS.exists(configPath)) {
        Serial.println("Run config not found: " + runName);
        return false;
    }
    
    // Read existing config
    File file = LittleFS.open(configPath, "r");
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false;
    }
    
    // Update notes
    doc["notes"] = notes;
    
    // Write back
    file = LittleFS.open(configPath, "w");
    if (!file) {
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    // Update current run if it's active
    if (runActive && currentRun.name == runName) {
        currentRun.notes = notes;
    }
    
    Serial.println("Updated notes for run: " + runName);
    return true;
}

void resetStartTime(unsigned long millis) {
    currentRun.startTime = millis;
}