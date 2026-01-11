#include "data_logger.h"
#include "config.h"
#include <LittleFS.h>

static File currentFile;
static String currentFileName = "";
static bool fileOpen = false;
static int sampleCount = 0;

bool initDataLogger() {
    // Create data directories if they don't exist
    if (!LittleFS.exists(DATA_DIR)) {
        if (!LittleFS.mkdir(DATA_DIR)) {
            Serial.println("Failed to create data directory");
            return false;
        }
    }
    
    if (!LittleFS.exists(RUNS_DIR)) {
        if (!LittleFS.mkdir(RUNS_DIR)) {
            Serial.println("Failed to create runs directory");
            return false;
        }
    }
    
    Serial.println("Data logger initialized");
    return true;
}

bool createDataFile(const String& fileName) {
    // Close any existing file
    if (fileOpen) {
        closeDataFile();
    }
    
    // Open new file for writing
    String fullPath = String(RUNS_DIR) + "/" + fileName;
    currentFile = LittleFS.open(fullPath, "w");
    
    if (!currentFile) {
        Serial.println("Failed to create data file: " + fullPath);
        return false;
    }

    // Write CSV header
    currentFile.println("timestamp_ms,thrust_grams");
    currentFile.flush();
    
    currentFileName = fullPath;
    fileOpen = true;
    sampleCount = 0;
    
    Serial.println("Created data file: " + fullPath);
    return true;
}

int getSampleCount() {
    return sampleCount;
}

bool logSample(float thrust, unsigned long timestamp) {
    if (!fileOpen || !currentFile) {
        Serial.println("No file open for logging");
        return false;
    }
    
    // Write CSV row: timestamp,thrust
    currentFile.print(timestamp);
    currentFile.print(",");
    currentFile.println(thrust, 2);  // 2 decimal places
    
    // Flush every 10 samples to balance performance and safety
    //static int sampleCount = 0;
    if (++sampleCount % 10 == 0) {
        currentFile.flush();
        //sampleCount = 0;
    }
    
    return true;
}

void closeDataFile() {
    if (fileOpen && currentFile) {
        currentFile.flush();
        currentFile.close();
        Serial.println("Closed data file: " + currentFileName);
    }
    fileOpen = false;
    currentFileName = "";
}

String readDataFile(const String& fileName) {
    String fullPath = fileName.startsWith("/") ? fileName : String(RUNS_DIR) + "/" + fileName;
    
    File file = LittleFS.open(fullPath, "r");
    if (!file) {
        Serial.println("Failed to open file for reading: " + fullPath);
        return "";
    }
    
    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    
    return content;
}

bool deleteDataFile(const String& fileName) {
    String fullPath = fileName.startsWith("/") ? fileName : String(RUNS_DIR) + "/" + fileName;
    
    if (!LittleFS.exists(fullPath)) {
        Serial.println("File does not exist: " + fullPath);
        return false;
    }
    
    if (LittleFS.remove(fullPath)) {
        Serial.println("Deleted file: " + fullPath);
        return true;
    }
    
    Serial.println("Failed to delete file: " + fullPath);
    return false;
}

size_t getFileSize(const String& fileName) {
    String fullPath = fileName.startsWith("/") ? fileName : String(RUNS_DIR) + "/" + fileName;
    
    File file = LittleFS.open(fullPath, "r");
    if (!file) {
        return 0;
    }
    
    size_t size = file.size();
    file.close();
    return size;
}

String listDataFiles(const String& directory) {
    String fileList = "[";
    
    File dir = LittleFS.open(directory);
    if (!dir || !dir.isDirectory()) {
        Serial.println("Failed to open directory: " + directory);
        return "[]";
    }
    
    File file = dir.openNextFile();
    bool first = true;
    
    while (file) {
        if (!file.isDirectory()) {
            if (!first) {
                fileList += ",";
            }
            fileList += "\"" + String(file.name()) + "\"";
            first = false;
        }
        file = dir.openNextFile();
    }
    
    fileList += "]";
    return fileList;
}

