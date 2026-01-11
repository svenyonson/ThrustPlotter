# Thrust Plotter
An ESP32 based appliance used to measure propeller/motor thrust over time. It uses a 1kg Load Cell to measure thrust.
You can learn everything you need to know about setting up load cells here:

https://randomnerdtutorials.com/esp32-load-cell-hx711/

But, don't worry about any of the software on that page, you will use this code instead. 

Mechanically, it is basically a digital gram scale, but we're not building a traditional scale - we need to measure sideways force. For this, you will need to build something that has a track, a wheeled platform that rolls along that track, and a place to mount the load cell such that the platform can push against the load cell. Then, on top of the rolling platform, you need to build something to hold the propeller and the rubber motor. 

![Thrust Plotter Appliance](<./assets/images/Thrust Plotter.jpg>)

I used a piece of extrusion for the rails and a mini-gantry for the platform - because I had them lying around. You can make something out of wood combined with some small wheels - try to minimize sliding friction if possible. I am including the design and STL files for the printed parts if you can use them.

## Hardware
- Mechanical frame/rail/platform as described above
- ESP32 micro controller
- 1kg Load Cell with HX711 amplifier 

## Wiring Diagram
![Wiring Diagram](<./assets/images/Wiring Diagram.png>)

## Toolchain
- Visual Studio Code
- PlatformIO (vscode plugin)

## Software
The ESP32 application has the following components/features:
- Captive Portal
  - The device can be placed in AP mode by holding the BOOT button down for 3 seconds. This allows for setting up the following:
    - WiFi SSID
    - WiFi password
    - Local timezone (for timestamped filenames)
    - Calibrating the load cell
- Web Application
  - Provides a tabbed user interface for all interactions with the appliance:
    - Creating run configurations (prop/motor/winds/other)
    - Start/Stop runs
    - Create thrust/time charts using one or more runs for comparisons
    - A primitive "explorer" so you can see what is on the onboard file system
    - An upload interface for updating the web application
- mDNS Implementation
    - Because the appliance has no display, we need a way to connect to the web application without knowing its IP address. Using mDNS, once the appliance is connected to wifi, you just point your browser to http://thrustplotter.local and the IP address is automatically resolved.
- Integrated Calibration
    - Normally, you would need to run a separate application, connected to an IDE, to perform the calibration step. This utility is built into the captive portal screen. You can re-run the calibration as often as you like.

### Project Structure
![Thrust Plotter Appliance](<./assets/images/Project Structure.png>)


### Build & Setup Instructions
- Prerequisites
  - Assemble the thrust plotter frame, rail, platform, etc
  - Complete wiring between the load cell, HX711, and the ESP32
  - Install Visual Studio Code
  - Install PlatformIO
  - Clone this repository
  - Connect the ESP32 with a USB (C) cable
- Open the project folder in vscode
- On the left toolbar, click on the alien icon (PlatformIO)
  - Click Upload and Monitor
    - The IDE console will show that wifi has not been setup yet. No blue LED on the ESP32 (just the red POWER led)
- Press and hold the BOOT button for 3 seconds. Observe blue LED flashing. This is the captive portal (setup mode)
- On your computer or phone, change your WiFi connection to ThrustPlotter-Config (no password)
- A popup window should appear:

- ![WiFi Config](<./assets/images/WiFi Config.png>)
- Enter the SSID, password, and timezone - then click Save and Connect (we'll come back for the calibration later)
- The popup is dismissed, the ESP32 reboots and connects to your WiFi, and your computer reconnects to its last WiFi SSID.
- If the ESP32 has successfully connected to your WiFi, the blue LED will be solid blue.
- Point your browser to http://thrustplotter.local. The IP address is resolved for you and you should be connected to the device. However, the web application has not yet been installed on the ESP32 file system (this is separate from the ESP32 application). Because index.html does not yet exist on the device, you are redirected to an upload page. Browse to the ./data/web folder, select index.html, and the click upload. 
- Refresh the web page and you should see the application screen. If you make changes to index.html, you can use the Upload tab in the application to upload index.html

### Load Cell Calibration
At this point you can use the appliance, but your readings will be way off.
- Find a small object of a known weight (weigh it on your kitchen scale)
- Turn the appliance on end so that you can place the weight on the load cell when directed to.
- ![Thrust Plotter Calibration Position](<./assets/images/Calibration Position.jpg>)
- Press and hold the BOOT button for 3 seconds. As before, the Thrust Plotter Config screen appears. 
- Click on the Calibration tab
- ![Thrust Plotter Calibration Screen](<./assets/images/Calibration Screen.png>)
- Enter the weight of the known object
- Click Start Calibration
- Follow the onscreen instructions
- Click Reboot
- If the ESP32 has successfully connected to your WiFi, the blue LED will be solid blue.

### Calibration Test
With the appliance still on its end, point your browser to http://thrustplotter.local. 
- In the Runs tab, create a new run configuration (name, notes). This is a throwaway, name it anything.
- You will see the new Run name under "Your Runs".
- Click Start
- Place the weight on the load cell
- Click Stop
- Click the Charts tab
- Select the run file that you just created
- Click Generate Chart
- Observe that the stable "thrust" is about equal to the weight of your object of known weight

### Operation
- Assemble a fixture to attach to the rolling platform to hold the propeller assembly and the rubber motor. 
- Wind 'er up and generate plots!

### Notes
- The plots are trimmed to remove zero/noise samples from the beginning of the plot (.csv) file. So take your time after you click start and try to gently release the prop.
- The files used to print the red plastic parts for mounting the propeller assembly and motor are included. Besides the STL files, the original Fusion files are there too if you want to modify them.
- The nose block and rear hook parts have a hole in the bottom for an M5 threaded insert (install with soldering iron to melt into place)
- The wood strip supporting the nose block and rear hook is 4mm x 15mm x 410mm


