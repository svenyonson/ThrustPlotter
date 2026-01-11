// config_page.h
#ifndef CONFIG_PAGE_H
#define CONFIG_PAGE_H

#include <pgmspace.h>

const char CONFIG_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Thrust Meter Configuration</title>
    <style>
        body { font-family: -apple-system, sans-serif; margin: 0; padding: 10px; background: #f4f4f9; font-size: 14px; }
        .container { 
            background: white; 
            padding: 15px; 
            border-radius: 8px; 
            box-shadow: 0 2px 8px rgba(0,0,0,0.1); 
            box-sizing: border-box;

            width: 90%;            
            max-width: 380px;      
            margin: 10px auto;     
        }
        h1 { color: #333; text-align: center; font-size: 1.2rem; margin: 0 0 15px 0; }
        
        .tabs { display: flex; border-bottom: 2px solid #e0e0e0; margin-bottom: 15px; }
        .tab { flex: 1; padding: 8px; text-align: center; cursor: pointer; background: #f5f5f5; border: none; font-weight: 600; color: #666; }
        .tab.active { background: white; color: #007bff; border-bottom: 3px solid #007bff; }
        
        .tab-content { display: none; }
        .tab-content.active { display: block; }
        
        label { display: block; margin-top: 10px; margin-bottom: 4px; color: #666; font-weight: 600; font-size: 0.85rem; }
        input[type='text'], input[type='password'], input[type='number'], select {
            width: 100%; padding: 8px; margin-bottom: 5px; border: 1px solid #ccc; border-radius: 6px; box-sizing: border-box; font-size: 14px;
        }
        .instructionsText {font-size: 0.65rem;}
        button { width: 100%; padding: 10px; background: #007bff; color: white; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; cursor: pointer; margin-top: 10px;}
        .calibration-value { font-size: 1rem; font-weight: bold; color: #28a745; text-align: center; padding: 10px; background: #f8f9fa; border-radius: 6px; margin: 10px 0; }
        .instructions { background: #e7f3ff; padding: 10px; border-radius: 6px; margin: 10px 0; border-left: 3px solid #007bff; font-size: 0.85rem; }
        .status { margin-top: 10px; padding: 8px; border-radius: 6px; text-align: center; font-size: 0.85rem; }
        .hidden { display: none; }
        .reboot-btn {
            position: fixed;
            bottom: 20px;
            right: 20px;
            width: auto; /* Overrides the 100% width of standard buttons */
            padding: 10px 15px;
            background: #dc3545; /* Red color for a 'system' action */
            font-size: 12px;
            margin-top: 0;
            box-shadow: 0 4px 10px rgba(0,0,0,0.2);
            z-index: 1000;
        }
        .reboot-btn:hover { background: #c82333; }
        .modal-overlay {
            display: none; /* Hidden by default */
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(0,0,0,0.6);
            z-index: 2000;
            align-items: center;
            justify-content: center;
        }

        /* The Modal Box */
        .modal-box {
            background: white;
            padding: 20px;
            border-radius: 12px;
            width: 85%;
            max-width: 300px;
            text-align: center;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
        }

        .modal-btns { display: flex; gap: 10px; margin-top: 20px; }
        .btn-confirm { background: #dc3545; color: white; border: none; padding: 10px; flex: 1; border-radius: 6px; font-weight: bold; }
        .btn-cancel { background: #6c757d; color: white; border: none; padding: 10px; flex: 1; border-radius: 6px; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>⚙️ Thrust Plotter Config</h1>
        
        <div class='tabs'>
            <button class='tab active' onclick='switchTab(0)'>WiFi</button>
            <button class='tab' onclick='switchTab(1)'>Calibration</button>
        </div>
        
        <!-- WiFi Tab -->
        <div class='tab-content active' id='tab0'>
            <form id='wifiForm'>
                <label for='ssid'>WiFi Network Name (SSID):</label>
                <input type='text' id='ssid' name='ssid' required placeholder='SSID'>
                
                <label for='password'>WiFi Password:</label>
                <input type='password' id='password' name='password' placeholder='Password (leave blank if unchanged)'>
                
                <label for='tz_select'>Local Timezone:</label>
                <select id='tz_select'>
                    <optgroup label="Americas">
                        <option value="AST4">Atlantic (No DST)</option>
                        <option value="EST5EDT,M3.2.0,M11.1.0">Eastern Time (US/Canada)</option>
                        <option value="CST6CDT,M3.2.0,M11.1.0">Central Time (US/Canada)</option>
                        <option value="MST7MDT,M3.2.0,M11.1.0">Mountain Time (US/Canada)</option>
                        <option value="MST7">Mountain Standard (No DST - Arizona)</option>
                        <option value="PST8PDT,M3.2.0,M11.1.0">Pacific Time (US/Canada)</option>
                        <option value="AKST9AKDT,M3.2.0,M11.1.0">Alaska</option>
                        <option value="HST10">Hawaii</option>
                    </optgroup>
                    <optgroup label="Europe">
                        <option value="GMT0BST,M3.5.0/1,M10.5.0">London / Dublin</option>
                        <option value="CET-1CEST,M3.5.0,M10.5.0">Paris / Berlin / Rome / Madrid</option>
                        <option value="EET-2EEST,M3.5.0,M10.5.0">Athens / Bucharest / Istanbul</option>
                        <option value="MSK-3">Moscow</option>
                    </optgroup>
                    <optgroup label="Asia/Oceania">
                        <option value="GST-4">Dubai</option>
                        <option value="IST-5.5">India (Mumbai/Kolkata)</option>
                        <option value="CST-8">China / Hong Kong / Singapore</option>
                        <option value="JST-9">Japan / Korea</option>
                        <option value="AWST-8">Australia (Perth)</option>
                        <option value="AEST-10AEDT,M10.1.0,M4.1.0/3">Australia (Sydney/Melbourne)</option>
                        <option value="NZST-12NZDT,M9.5.0,M4.1.0/3">New Zealand</option>
                    </optgroup>
                    <optgroup label="Other">
                        <option value="GMT0">UTC / GMT</option>
                    </optgroup>
                </select>
                <input type="hidden" id="timezone" name="timezone">
                
                <button type='submit'>Save and Connect</button>
            </form>
            <div id='wifiStatus' class='status hidden'></div>
        </div>
        
        <!-- Calibration Tab -->
        <div class='tab-content' id='tab1'>
            <div id='calibrationDisplay' class='calibration-value'>
                Calibration Factor: <span id='currentCalibration'>Loading...</span>
            </div>
            
            <div id='calibrationForm'>
                <label for='knownWeight'>Known Weight (grams):</label>
                <input type='number' id='knownWeight' step='0.1' min='1' placeholder='e.g., 100.0' required>
                
                <button type='button' id='calibrateBtn' class='btn-secondary'>Start Calibration</button>
            </div>
            
            <div id='calibrationInstructions' class='instructions hidden'>
                <strong>Calibration Steps:</strong>
                <ol class='instructionsText'>
                    <li>Remove all weight from the load cell, then wait for tare to complete (5 seconds)</li>
                    <li>Place your known weight on the load cell, then wait for measurements to complete</li>
                    <li>Calibration factor will be calculated and saved</li>
                </ol>
            </div>
            
            <div id='calibrationProgress' class='progress hidden'></div>
            <div id='calibrationStatus' class='status hidden'></div>
        </div>
    </div>
    
    <script>
        // Tab switching
        function switchTab(index) {
            document.querySelectorAll('.tab').forEach((tab, i) => {
                tab.classList.toggle('active', i === index);
            });
            document.querySelectorAll('.tab-content').forEach((content, i) => {
                content.classList.toggle('active', i === index);
            });
        }
        
        // Load current settings on page load
        fetch('/api/config')
            .then(response => response.json())
            .then(data => {
                // Prefill WiFi settings
                if (data.ssid) {
                    document.getElementById('ssid').value = data.ssid;
                }
                if (data.timezone) {
                    document.getElementById('tz_select').value = data.timezone;
                }
                
                // Show calibration factor
                if (data.calibrationFactor && data.calibrationFactor !== 0) {
                    document.getElementById('currentCalibration').textContent = data.calibrationFactor.toFixed(2);
                } else {
                    document.getElementById('currentCalibration').textContent = 'Not calibrated';
                }
            })
            .catch(err => {
                console.error('Error loading config:', err);
                document.getElementById('currentCalibration').textContent = 'Error loading';
            });

        // WiFi form submission
        document.getElementById('wifiForm').addEventListener('submit', function(e) {
            e.preventDefault();
            
            document.getElementById('timezone').value = document.getElementById('tz_select').value;
            
            var formData = new FormData(this);
            var status = document.getElementById('wifiStatus');
            
            fetch('/save', {
                method: 'POST',
                body: new URLSearchParams(formData)
            })
            .then(response => {
                if(response.ok) {
                    status.textContent = 'Settings saved! Restarting...';
                    status.className = 'status success';
                } else {
                    throw new Error();
                }
                status.classList.remove('hidden');
            })
            .catch(error => {
                status.textContent = 'Error saving settings.';
                status.className = 'status error';
                status.classList.remove('hidden');
            });
        });

        // Calibration
        let calibrationTimer = null;

        document.getElementById('calibrateBtn').addEventListener('click', function() {
            const knownWeight = parseFloat(document.getElementById('knownWeight').value);
            
            if (!knownWeight || knownWeight <= 0) {
                showCalibrationStatus('Please enter a valid weight', 'error');
                return;
            }

            startCalibration(knownWeight);
        });

        function startCalibration(knownWeight) {
            const btn = document.getElementById('calibrateBtn');
            const instructions = document.getElementById('calibrationInstructions');
            const progress = document.getElementById('calibrationProgress');
            
            btn.disabled = true;
            instructions.classList.remove('hidden');
            progress.classList.remove('hidden');
            
            updateProgress('Starting calibration...');
            
            fetch('/api/calibration/start', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({knownWeight: knownWeight})
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    pollCalibrationStatus();
                } else {
                    showCalibrationStatus('Failed to start calibration', 'error');
                    resetCalibrationUI();
                }
            })
            .catch(error => {
                showCalibrationStatus('Error: ' + error, 'error');
                resetCalibrationUI();
            });
        }

        function pollCalibrationStatus() {
            calibrationTimer = setInterval(() => {
                fetch('/api/calibration/status')
                    .then(response => response.json())
                    .then(data => {
                        if (data.step) {
                            updateProgress(data.message);
                        }
                        
                        if (data.complete) {
                            clearInterval(calibrationTimer);
                            
                            if (data.success) {
                                document.getElementById('currentCalibration').textContent = data.calibrationFactor.toFixed(2);
                                showCalibrationStatus('Calibration complete! Factor: ' + data.calibrationFactor.toFixed(2), 'success');
                            } else {
                                showCalibrationStatus('Calibration failed: ' + data.message, 'error');
                            }
                            
                            resetCalibrationUI();
                        }
                    })
                    .catch(error => {
                        clearInterval(calibrationTimer);
                        showCalibrationStatus('Error polling status', 'error');
                        resetCalibrationUI();
                    });
            }, 1000);
        }

        function updateProgress(message) {
            document.getElementById('calibrationProgress').textContent = message;
        }

        function showCalibrationStatus(message, type) {
            const status = document.getElementById('calibrationStatus');
            status.textContent = message;
            status.className = 'status ' + type;
            status.classList.remove('hidden');
        }

        function resetCalibrationUI() {
            document.getElementById('calibrateBtn').disabled = false;
            document.getElementById('calibrationProgress').classList.add('hidden');
            document.getElementById('calibrationInstructions').classList.add('hidden');
        }
        function rebootDevice() {
            if (confirm("Are you sure you want to reboot the device? Connection will be lost.")) {
                fetch('/api/reboot', { method: 'POST' })
                    .then(() => {
                        alert("Rebooting... You can close this window.");
                    })
                    .catch(err => {
                        alert("Reboot command sent. Device is restarting.");
                    });
            }
        }
        function showRebootModal() {
            document.getElementById('rebootModal').style.display = 'flex';
        }

        function hideRebootModal() {
            document.getElementById('rebootModal').style.display = 'none';
        }

        function executeReboot() {
            // Hide the buttons so they can't click twice
            document.querySelector('.modal-btns').innerHTML = '<p>Restarting...</p>';
            
            fetch('/api/reboot', { method: 'POST' })
                .then(() => {
                    console.log("Rebooting...");
                })
                .catch(err => {
                    console.log("Device rebooting, fetch failed as expected.");
                });

            // Optional: Give the user a visual cue before they manually close the window
            setTimeout(() => {
                document.getElementById('rebootModal').innerHTML = 
                    "<div class='modal-box'><h3>Rebooting...</h3><p>You can now close this window and reconnect in 10 seconds.</p></div>";
            }, 1000);
        }
    </script>
    <button class='reboot-btn' onclick='showRebootModal()'>🔄 Reboot</button>

    <div id='rebootModal' class='modal-overlay'>
        <div class='modal-box'>
            <h3>Reboot Device?</h3>
            <p>Connection to the captive portal will be lost and your device will reconnect to your configured SSID.</p>
            <div class='modal-btns'>
                <button class='btn-cancel' onclick='hideRebootModal()'>Cancel</button>
                <button class='btn-confirm' onclick='executeReboot()'>Reboot</button>
            </div>
        </div>
    </div>
    </body>
</html>
)=====";

#endif