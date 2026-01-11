// src/web_server.cpp
#include "web_server.h"
#include "config.h"
#include "run_manager.h"
#include "data_logger.h"
#include "chart_manager.h"
#include "upload_page.h"
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// This is not static because it is externed in wifi_manager.cpp
WebServer server(WEB_SERVER_PORT);

// Forward declarations
void handleFileUpload();
void handleFileUploadComplete();
void handleNotFound();
void handleRoot();
void handleUploadPage();
void handleRunsAPI(String path);
void handleDataAPI(String path);
void handleChartsAPI(String path);
void handleGetRuns();
void handleCreateRun();
void handleUpdateRun();
void handleUpdateRunWithName(const String& runName);
void handleDeleteRun();
void handleDeleteRunWithName(const String& runName);
void handleStartRun();
void handleStartRunWithName(const String& runName);
void handleStopRun();
void handleGetRunFiles();
void handleGetRunFilesWithName(const String& runName);
void handleGetCurrentRun();
void handleGetDataFile();
void handleGetDataFileWithName(const String& fileName);
void handleDeleteDataFile();
void handleDeleteDataFileWithName(const String& fileName);
void handleGetCharts();
void handleCreateChart();
void handleGetChart();
void handleGetChartWithName(const String& chartName);
void handleDeleteChart();
void handleDeleteChartWithName(const String& chartName);
void handleGenerateChartData();
void handleExportChart();
void handleListFiles();

bool initWebServer() {
    // Create web directory if it doesn't exist
    if (!LittleFS.exists("/web")) {
        if (!LittleFS.mkdir("/web")) {
            Serial.println("Failed to create web directory");
            return false;
        }
    }
    
    // Serve root
    server.on("/", HTTP_GET, handleRoot);
    
    // Dedicated upload page endpoint (always accessible)
    server.on("/upload", HTTP_GET, handleUploadPage);
    
    // File upload endpoint
    server.on("/upload", HTTP_POST, handleFileUploadComplete, handleFileUpload);
    
    // Run management API
    server.on("/api/runs", HTTP_GET, handleGetRuns);
    server.on("/api/runs", HTTP_POST, handleCreateRun);
    server.on("/api/runs/current", HTTP_GET, handleGetCurrentRun);
    server.on("/api/runs/stop", HTTP_POST, handleStopRun);
    
    // Chart API
    //server.on("/api/charts", HTTP_GET, handleGetCharts);
    //server.on("/api/charts", HTTP_POST, handleCreateChart);
    server.on("/api/charts/data", HTTP_POST, handleGenerateChartData);
    //server.on("/api/charts/export", HTTP_POST, handleExportChart);
    
    // File list endpoint for upload page
    server.on("/api/files", HTTP_GET, []() {
        File dir = LittleFS.open("/web");
        String files = "[";
        if (dir && dir.isDirectory()) {
            File file = dir.openNextFile();
            bool first = true;
            while (file) {
                if (!file.isDirectory()) {
                    if (!first) files += ",";
                    files += "\"" + String(file.name()) + "\"";
                    first = false;
                }
                file = dir.openNextFile();
            }
        }
        files += "]";
        server.send(200, "application/json", files);
    });
    
    // Serve static files from /web directory
    server.onNotFound(handleNotFound);

    // Explorer endpoints
        server.on("/listfiles", HTTP_GET, handleListFiles);
    // --- DOWNLOAD HANDLER ---
    server.on("/download", HTTP_GET, []() {
        String path = server.arg("path");
        if (path == "" || !LittleFS.exists(path)) {
            server.send(404, "text/plain", "File Not Found");
            return;
        }
        
        File file = LittleFS.open(path, "r");
        if (file.isDirectory()) {
            server.send(403, "text/plain", "Cannot download a directory");
            return;
        }

        // This streams the file directly to the browser
        server.streamFile(file, "application/octet-stream");
        file.close();
    });

    // --- DELETE HANDLER ---
    server.on("/delete", HTTP_DELETE, []() {
        String path = server.arg("path");
        if (path == "" || !LittleFS.exists(path)) {
            server.send(404, "text/plain", "Path Not Found");
            return;
        }

        // LittleFS.remove() works for files, LittleFS.rmdir() for folders
        // Note: rmdir only works if the directory is empty!
        if (LittleFS.remove(path) || LittleFS.rmdir(path)) {
            server.send(200, "text/plain", "Deleted");
        } else {
            server.send(500, "text/plain", "Delete Failed (Folder might not be empty)");
        }
    });

    // --- RENAME HANDLER ---
    server.on("/rename", HTTP_POST, []() {
        String oldPath = server.arg("old");
        String newPath = server.arg("new");
        
        if (oldPath == "" || newPath == "") {
            server.send(400, "text/plain", "Missing Paths");
            return;
        }

        if (LittleFS.rename(oldPath, newPath)) {
            server.send(200, "text/plain", "Renamed");
        } else {
            server.send(500, "text/plain", "Rename Failed");
        }
    });

    server.begin();
    Serial.println("Web server started on port " + String(WEB_SERVER_PORT));
    return true;
}

void handleWebServer() {
    server.handleClient();
}

// Serve files from /web directory or show upload page
void handleRoot() {
    // Check if index.html exists
    if (LittleFS.exists("/web/index.html")) {
        File file = LittleFS.open("/web/index.html", "r");
        server.streamFile(file, "text/html");
        file.close();
    } else {
        // Show file upload interface
        server.send(200, "text/html", UPLOAD_PAGE);
    }
}

// Always show upload page
void handleUploadPage() {
    server.send(200, "text/html", UPLOAD_PAGE);
}

File uploadFile;

void handleFileUpload() {
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        String filename = "/web/" + String(upload.filename);
        Serial.println("Upload start: " + filename);
        uploadFile = LittleFS.open(filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
            Serial.println("Upload complete: " + String(upload.totalSize) + " bytes");
        }
    }
}

void handleFileUploadComplete() {
    server.send(200, "text/plain", "OK");
}

void handleNotFound() {
    String path = server.uri();
    
    // Handle API routes with path parameters
    if (path.startsWith("/api/runs/")) {
        handleRunsAPI(path);
        return;
    } else if (path.startsWith("/api/data/")) {
        handleDataAPI(path);
        return;
    } /*else if (path.startsWith("/api/charts/")) {
        handleChartsAPI(path);
        return;
    }
    */
    Serial.println("handleNotFound called for: " + path + " Method: "+server.method());

    // Try to serve from /web directory
    String webPath = "/web" + path;
    
    if (LittleFS.exists(webPath)) {
        File file = LittleFS.open(webPath, "r");
        String contentType = "text/plain";
        
        if (path.endsWith(".html")) contentType = "text/html";
        else if (path.endsWith(".css")) contentType = "text/css";
        else if (path.endsWith(".js")) contentType = "application/javascript";
        else if (path.endsWith(".json")) contentType = "application/json";
        else if (path.endsWith(".png")) contentType = "image/png";
        else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) contentType = "image/jpeg";
        else if (path.endsWith(".ico")) contentType = "image/x-icon";
        
        server.streamFile(file, contentType);
        file.close();
    } else {
        Serial.println("File not found: " + webPath);
        server.send(404, "text/plain", "Not Found: " + path);
    }
}

// Handle /api/runs/* routes
void handleRunsAPI(String path) {
    // Remove /api/runs/ prefix
    String remainder = path.substring(10);
    
    if (remainder == "current") {
        if (server.method() == HTTP_GET) {
            handleGetCurrentRun();
        }
    } else if (remainder == "stop") {
        if (server.method() == HTTP_POST) {
            handleStopRun();
        }
    } else {
        // Extract run name and decode URL encoding
        int slashPos = remainder.indexOf('/');
        String runName = (slashPos > 0) ? remainder.substring(0, slashPos) : remainder;
        String action = (slashPos > 0) ? remainder.substring(slashPos + 1) : "";
        
        // URL decode the run name (replace %20 with spaces, etc)
        runName.replace("%20", " ");
        runName.replace("%21", "!");
        runName.replace("%23", "#");
        runName.replace("%24", "$");
        runName.replace("%26", "&");
        runName.replace("%27", "'");
        runName.replace("%28", "(");
        runName.replace("%29", ")");
        runName.replace("%2B", "+");
        runName.replace("%2C", ",");
        runName.replace("%2F", "/");
        runName.replace("%3A", ":");
        runName.replace("%3B", ";");
        runName.replace("%3D", "=");
        runName.replace("%3F", "?");
        runName.replace("%40", "@");
        runName.replace("%5B", "[");
        runName.replace("%5D", "]");
        
        //Serial.println("Decoded run name: " + runName);
        
        if (action == "start" && server.method() == HTTP_POST) {
            handleStartRunWithName(runName);
        } else if (action == "files" && server.method() == HTTP_GET) {
            handleGetRunFilesWithName(runName);
        } else if (action == "" && server.method() == HTTP_PUT) {
            handleUpdateRunWithName(runName);
        } else if (action == "" && server.method() == HTTP_DELETE) {
            handleDeleteRunWithName(runName);
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    }
}

// Handle /api/data/* routes
void handleDataAPI(String path) {
    String fileName = path.substring(10); // Remove /api/data/
    
    // URL decode the filename
    fileName.replace("%20", " ");
    fileName.replace("%21", "!");
    fileName.replace("%23", "#");
    fileName.replace("%24", "$");
    fileName.replace("%26", "&");
    fileName.replace("%27", "'");
    fileName.replace("%28", "(");
    fileName.replace("%29", ")");
    fileName.replace("%2B", "+");
    fileName.replace("%2C", ",");
    fileName.replace("%2F", "/");
    
    Serial.println("Decoded file name: " + fileName);
    
    if (server.method() == HTTP_GET) {
        handleGetDataFileWithName(fileName);
    } else if (server.method() == HTTP_DELETE) {
        handleDeleteDataFileWithName(fileName);
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}

// Handle /api/charts/* routes
/*
void handleChartsAPI(String path) {
    String chartName = path.substring(12); // Remove /api/charts/
    
    // URL decode the chart name
    chartName.replace("%20", " ");
    chartName.replace("%21", "!");
    chartName.replace("%23", "#");
    chartName.replace("%24", "$");
    chartName.replace("%26", "&");
    chartName.replace("%27", "'");
    chartName.replace("%28", "(");
    chartName.replace("%29", ")");
    chartName.replace("%2B", "+");
    chartName.replace("%2C", ",");
    
    Serial.println("Decoded chart name: " + chartName);
    
    if (server.method() == HTTP_GET) {
        handleGetChartWithName(chartName);
    } else if (server.method() == HTTP_DELETE) {
        handleDeleteChartWithName(chartName);
    } else {
        server.send(404, "text/plain", "Not Found");
    }
}
*/

// API Handlers

void handleGetRuns() {
    String json = getRunConfigsList();
    server.send(200, "application/json", json);
}

void handleCreateRun() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }

    Serial.println("POST body: " + server.arg("plain"));
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    String name = doc["name"].as<String>();
    String notes = doc["notes"].as<String>();
    
    if (createRunConfig(name, notes)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to create run");
    }
}

void handleUpdateRun() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    String runName = doc["name"].as<String>();
    String notes = doc["notes"].as<String>();
    
    if (updateRunNotes(runName, notes)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to update run");
    }
}

void handleUpdateRunWithName(const String& runName) {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    String notes = doc["notes"].as<String>();
    
    if (updateRunNotes(runName, notes)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to update run");
    }
}

void handleDeleteRun() {
    server.send(400, "text/plain", "Run name required");
}

void handleDeleteRunWithName(const String& runName) {
    if (deleteRun(runName)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to delete run");
    }
}

void handleStartRun() {
    server.send(400, "text/plain", "Run name required");
}

void handleStartRunWithName(const String& runName) {
    if (startRun(runName)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to start run");
    }
}

void handleGetRunFiles() {
    server.send(400, "text/plain", "Run name required");
}

void handleGetRunFilesWithName(const String& runName) {
    String json = getRunDataFiles(runName);
    server.send(200, "application/json", json);
}

void handleGetDataFile() {
    server.send(400, "text/plain", "Filename required");
}

void handleGetDataFileWithName(const String& fileName) {
    String content = readDataFile(fileName);
    
    if (content.length() > 0) {
        server.send(200, "text/csv", content);
    } else {
        server.send(404, "text/plain", "File not found");
    }
}

void handleDeleteDataFile() {
    server.send(400, "text/plain", "Filename required");
}

void handleDeleteDataFileWithName(const String& fileName) {
    if (deleteDataFile(fileName)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to delete file");
    }
}
/*
void handleGetChart() {
    server.send(400, "text/plain", "Chart name required");
}

void handleGetChartWithName(const String& chartName) {
    ChartConfig config = loadChartConfig(chartName);
    
    if (config.fileCount == 0) {
        server.send(404, "text/plain", "Chart not found");
        return;
    }
    
    JsonDocument doc;
    doc["name"] = config.name;
    doc["title"] = config.title;
    doc["description"] = config.description;
    doc["createdDate"] = config.createdDate;
    
    JsonArray files = doc["dataFiles"].to<JsonArray>();
    for (int i = 0; i < config.fileCount; i++) {
        files.add(config.dataFiles[i]);
    }
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void handleDeleteChart() {
    server.send(400, "text/plain", "Chart name required");
}

void handleDeleteChartWithName(const String& chartName) {
    if (deleteChartConfig(chartName)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to delete chart");
    }
}
*/
void handleStopRun() {
    if (stopRun()) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to stop run");
    }
}

void handleGetCurrentRun() {
    RunConfig run = getCurrentRun();
    
    JsonDocument doc;
    doc["name"] = run.name;
    doc["notes"] = run.notes;
    doc["isActive"] = run.isActive;
    doc["startTime"] = run.startTime;
    doc["currentFileName"] = run.currentFileName;
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

/*
void handleGetCharts() {
    String json = getChartsList();
    server.send(200, "application/json", json);
}

void handleCreateChart() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    ChartConfig config;
    config.name = doc["name"].as<String>();
    config.title = doc["title"].as<String>();
    config.description = doc["description"].as<String>();
    config.createdDate = doc["createdDate"].as<String>();
    
    JsonArray files = doc["dataFiles"];
    config.fileCount = 0;
    for (JsonVariant file : files) {
        if (config.fileCount < 10) {
            config.dataFiles[config.fileCount++] = file.as<String>();
        }
    }
    
    if (saveChartConfig(config)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "text/plain", "Failed to save chart");
    }
}
*/
void handleGenerateChartData() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Missing body");
        return;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    JsonArray filesArray = doc["files"];
    int fileCount = filesArray.size();
    
    if (fileCount == 0 || fileCount > 10) {
        server.send(400, "text/plain", "Invalid file count");
        return;
    }
    
    String fileNames[10];
    for (int i = 0; i < fileCount; i++) {
        fileNames[i] = filesArray[i].as<String>();
    }
    
    String chartData = generateChartData(fileNames, fileCount);
    server.send(200, "application/json", chartData);
}
/*
void handleExportChart() {
    // TODO: Implement ZIP export functionality
    // This requires a ZIP library or manual ZIP creation
    server.send(501, "text/plain", "Export not yet implemented");
}
*/
void streamFileJson(WebServer &server, String path) {
    File root = LittleFS.open(path);
    if (!root || !root.isDirectory()) return;

    server.sendContent("["); 
    File file = root.openNextFile();
    bool first = true;

    while (file) {
        if (!first) server.sendContent(",");
        
        server.sendContent("{");
        server.sendContent("\"name\":\"" + String(file.name()) + "\",");
        
        if (file.isDirectory()) {
            server.sendContent("\"type\":\"directory\",");
            server.sendContent("\"contents\":");
            // Recursion works perfectly with sendContent
            streamFileJson(server, String(file.path())); 
        } else {
            server.sendContent("\"type\":\"file\",");
            server.sendContent("\"size\":" + String(file.size()));
        }
        server.sendContent("}");
        
        file = root.openNextFile();
        first = false;
    }
    server.sendContent("]");
}

// Inside your handler function:
void handleListFiles() {
    // 1. Send headers and start 'chunked' encoding
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "application/json", ""); 

    // 2. Stream the JSON bit by bit
    streamFileJson(server, "/");
    
    // 3. Send an empty string to signal the end of the response
    server.sendContent(""); 
}
