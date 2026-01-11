#ifndef RUN_MANAGER_H
#define RUN_MANAGER_H

#include <Arduino.h>

struct RunConfig {
    String name;
    String notes;
    bool isActive;
    unsigned long startTime;
    String currentFileName;
};

// Initialize run manager and filesystem
bool initRunManager();

// Create a new run configuration
bool createRunConfig(const String& name, const String& notes);

// Start a run (creates CSV file and begins logging)
bool startRun(const String& runName);

// Stop the current run
bool stopRun();

// Delete a run configuration and all its data files
bool deleteRun(const String& runName);

// Get current run status
RunConfig getCurrentRun();

// Check if a run is currently active
bool isRunActive();

// Get list of all run configurations as JSON
String getRunConfigsList();

// Get list of all data files for a run
String getRunDataFiles(const String& runName);

// Update run notes
bool updateRunNotes(const String& runName, const String& notes);

// Reset StartTime - this is so we can trim zeros off the front of the file.
void resetStartTime(unsigned long millis);

#endif
