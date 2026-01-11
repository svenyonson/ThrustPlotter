#include "chart_manager.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

String generateChartData(const String* fileNames, int fileCount) {
    JsonDocument doc;
    JsonArray datasets = doc["datasets"].to<JsonArray>();
    
    for (int i = 0; i < fileCount; i++) {
        String filePath = String(RUNS_DIR) + "/" + fileNames[i];
        
        File file = LittleFS.open(filePath, "r");
        if (!file) {
            Serial.println("Failed to open file: " + filePath);
            continue;
        }
        
        JsonObject dataset = datasets.add<JsonObject>();
        dataset["name"] = fileNames[i];
        JsonArray data = dataset["data"].to<JsonArray>();
        
        // Skip header line
        String line = file.readStringUntil('\n');
        
        // Read data points
        while (file.available()) {
            line = file.readStringUntil('\n');
            line.trim();
            
            if (line.length() == 0) continue;
            
            int commaPos = line.indexOf(',');
            if (commaPos > 0) {
                JsonArray point = data.add<JsonArray>();
                point.add(line.substring(0, commaPos).toFloat());  // timestamp
                point.add(line.substring(commaPos + 1).toFloat()); // thrust
            }
        }
        
        file.close();
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

