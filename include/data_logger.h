#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>

// Initialize data logger
bool initDataLogger();

// Create a new CSV file for logging
bool createDataFile(const String& fileName);

// Log a thrust sample to the current file
bool logSample(float thrust, unsigned long timestamp);

// Close the current data file
void closeDataFile();

// Read CSV file contents
String readDataFile(const String& fileName);

// Delete a data file
bool deleteDataFile(const String& fileName);

// Get file size
size_t getFileSize(const String& fileName);

// List all CSV files in a directory
String listDataFiles(const String& directory);

// Get sample count
int getSampleCount();

#endif

