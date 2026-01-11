#ifndef CHART_MANAGER_H
#define CHART_MANAGER_H

#include <Arduino.h>

// Generate chart data JSON from multiple CSV files
String generateChartData(const String* fileNames, int fileCount);

#endif
