// ===== Project Structure =====
/*
project_root/
├── platformio.ini
├── data/                      # Files to upload to LittleFS
│   └── web/                   # Web application files
│       ├── index.html
│       ├── chart.html
│       └── style.css
├── include/
│   ├── config.h              # Global configuration constants
│   ├── wifi_manager.h        # WiFi configuration (existing)
│   ├── config_page.h         # WiFi config page HTML (existing)
│   ├── load_cell.h           # HX711 load cell interface
│   ├── run_manager.h         # Run configuration and control
│   ├── data_logger.h         # CSV file operations
│   ├── web_server.h          # Main web application server
│   └── chart_manager.h       # Chart configuration storage
└── src/
    ├── main.cpp              # Main program
    ├── wifi_manager.cpp      # WiFi logic (existing)
    ├── load_cell.cpp         # Load cell reading
    ├── run_manager.cpp       # Run lifecycle management
    ├── data_logger.cpp       # Data logging to CSV
    ├── web_server.cpp        # Web server and API endpoints
    └── chart_manager.cpp     # Chart config persistence
*/