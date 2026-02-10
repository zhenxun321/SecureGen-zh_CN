#pragma once

const char wifi_setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Setup - T-Display TOTP</title>
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <link rel="alternate icon" href="/favicon.ico">
    <style>
        @keyframes gradient-animation {
            0% { background-position: 0% 50%; }
            50% { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
        }

        @keyframes pulse {
            0% { transform: scale(1); opacity: 1; }
            50% { transform: scale(1.05); opacity: 0.8; }
            100% { transform: scale(1); opacity: 1; }
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: linear-gradient(-45deg, #1a1a2e, #16213e, #0f3460, #2e4a62);
            background-size: 400% 400%;
            animation: gradient-animation 15s ease infinite;
            color: #e0e0e0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            padding: 20px;
            box-sizing: border-box;
        }

        .container {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            padding: 2.5rem;
            border-radius: 15px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
            width: 100%;
            max-width: 450px;
            text-align: center;
        }

        h2 {
            margin-bottom: 2rem;
            color: #ffffff;
            font-weight: 300;
            letter-spacing: 1px;
            font-size: 1.8rem;
        }

        .input-group {
            margin-bottom: 1.5rem;
            text-align: left;
        }

        label {
            display: block;
            margin-bottom: 0.5rem;
            color: #b0b0b0;
            font-size: 0.9rem;
            font-weight: 500;
        }

        select, input[type="password"] {
            width: 100%;
            padding: 0.8rem;
            background-color: rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(255, 255, 255, 0.2);
            color: #e0e0e0;
            border-radius: 8px;
            box-sizing: border-box;
            transition: all 0.3s ease;
            font-size: 1rem;
        }

        select:focus, input:focus {
            outline: none;
            border-color: #5a9eee;
            box-shadow: 0 0 0 3px rgba(90, 158, 238, 0.3);
        }

        select option {
            background-color: #2a2a2a;
            color: #e0e0e0;
            padding: 8px;
        }

        /* Password visibility toggle */
        .password-input-container {
            position: relative;
            display: inline-block;
            width: 100%;
        }

        .password-toggle {
            position: absolute;
            right: 12px;
            top: 50%;
            transform: translateY(-50%);
            cursor: pointer;
            color: #b0b0b0;
            font-size: 1.2rem;
            user-select: none;
            transition: color 0.3s ease;
        }

        .password-toggle:hover {
            color: #5a9eee;
        }

        button {
            width: 100%;
            padding: 0.9rem;
            background-color: #5a9eee;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 600;
            transition: all 0.3s ease;
            margin-top: 1rem;
        }

        button:hover {
            background-color: #4a8bdb;
            transform: translateY(-1px);
        }

        button:active {
            transform: translateY(0px);
        }

        button:disabled {
            background-color: #555;
            color: #888;
            cursor: not-allowed;
            transform: none;
        }

        .loading {
            display: none;
            text-align: center;
            margin-top: 1rem;
            color: #5a9eee;
        }

        .network-info {
            font-size: 0.8rem;
            color: #888;
            margin-top: 0.3rem;
        }

        .refresh-btn {
            background: none;
            border: 1px solid rgba(255, 255, 255, 0.2);
            color: #b0b0b0;
            padding: 0.5rem 1rem;
            font-size: 0.9rem;
            margin-bottom: 1rem;
            width: auto;
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            cursor: pointer;
            transition: all 0.3s ease;
        }

        .refresh-btn:hover {
            border-color: #5a9eee;
            color: #5a9eee;
            background: rgba(90, 158, 238, 0.1);
        }

        .status-message {
            padding: 1rem;
            border-radius: 8px;
            margin-bottom: 1rem;
            text-align: center;
            display: none;
        }

        .status-message.error {
            background: rgba(244, 67, 54, 0.2);
            border: 1px solid rgba(244, 67, 54, 0.3);
            color: #e57373;
        }

        .status-message.success {
            background: rgba(76, 175, 80, 0.2);
            border: 1px solid rgba(76, 175, 80, 0.3);
            color: #81c784;
        }

        .network-strength {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-left: 8px;
        }
        
        .strength-excellent { background-color: #4CAF50; }
        .strength-good { background-color: #8BC34A; }
        .strength-fair { background-color: #FFC107; }
        .strength-poor { background-color: #FF9800; }
        .strength-weak { background-color: #F44336; }
    </style>
</head>
<body>
    <div class="container">
        <h2>🛜 WiFi Setup</h2>
        
        <div id="status-message" class="status-message"></div>
        
        <form id="wifi-form" action="/save" method="POST">
            <div class="input-group">
                <label for="ssid">Select Network</label>
                <button type="button" class="refresh-btn" onclick="scanNetworks()">🔄 Refresh Networks</button>
                <select id="ssid" name="ssid" required>
                    <option value="">Scanning networks...</option>
                </select>
            </div>
            
            <div class="input-group">
                <label for="password">WiFi Password</label>
                <div class="password-input-container">
                    <input type="password" id="password" name="password" placeholder="Enter network password">
                    <span class="password-toggle" onclick="togglePasswordVisibility('password', this)">👁</span>
                </div>
                <div class="network-info">Leave empty for open networks</div>
            </div>
            
            <button type="submit" id="submit-btn">💾 Save and Connect</button>
        </form>
        
        <div id="loading" class="loading">
            <div>⏳ Connecting to network...</div>
            <div style="font-size: 0.8rem; margin-top: 0.5rem;">This may take up to 30 seconds</div>
        </div>
    </div>

    <script>
        // Password visibility toggle function
        function togglePasswordVisibility(inputId, toggleElement) {
            const passwordInput = document.getElementById(inputId);
            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                toggleElement.textContent = '🙈';
            } else {
                passwordInput.type = 'password';
                toggleElement.textContent = '👁';
            }
        }

        function getSignalStrength(rssi) {
            if (rssi >= -40) return 'excellent';
            if (rssi >= -55) return 'good';
            if (rssi >= -70) return 'fair';
            if (rssi >= -85) return 'poor';
            return 'weak';
        }

        function showStatus(message, isError = false) {
            const statusDiv = document.getElementById('status-message');
            statusDiv.textContent = message;
            statusDiv.className = 'status-message ' + (isError ? 'error' : 'success');
            statusDiv.style.display = 'block';
            
            if (!isError) {
                setTimeout(() => {
                    statusDiv.style.display = 'none';
                }, 5000);
            }
        }

        function scanNetworks() {
            const select = document.getElementById('ssid');
            const refreshBtn = document.querySelector('.refresh-btn');
            
            select.innerHTML = '<option value="">Scanning networks...</option>';
            refreshBtn.disabled = true;
            refreshBtn.innerHTML = '⏳ Scanning...';
            
            fetch('/scan')
            .then(response => {
                if (!response.ok) throw new Error('Network scan failed');
                return response.json();
            })
            .then(data => {
                select.innerHTML = '';
                
                if (data.length === 0) {
                    select.innerHTML = '<option value="">No networks found</option>';
                    showStatus('No WiFi networks found. Try refreshing.', true);
                } else {
                    data.forEach(net => {
                        const option = document.createElement('option');
                        option.value = net.ssid;
                        const strength = getSignalStrength(net.rssi);
                        const strengthIcon = {
                            'excellent': '📶',
                            'good': '📶',
                            'fair': '📶',
                            'poor': '📶',
                            'weak': '📶'
                        }[strength];
                        
                        option.innerHTML = `${net.ssid} ${strengthIcon} (${net.rssi}dBm)`;
                        select.appendChild(option);
                    });
                    showStatus(`Found ${data.length} networks`);
                }
            })
            .catch(err => {
                select.innerHTML = '<option value="">Scan failed - try again</option>';
                showStatus('Failed to scan networks: ' + err.message, true);
            })
            .finally(() => {
                refreshBtn.disabled = false;
                refreshBtn.innerHTML = '🔄 Refresh Networks';
            });
        }

        // Form submission
        document.getElementById('wifi-form').addEventListener('submit', function(e) {
            e.preventDefault();
            
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            if (!ssid) {
                showStatus('Please select a network', true);
                return;
            }
            
            const submitBtn = document.getElementById('submit-btn');
            const form = document.getElementById('wifi-form');
            const loading = document.getElementById('loading');
            
            // Show loading state
            form.style.display = 'none';
            loading.style.display = 'block';
            
            // Submit form data
            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);
            
            fetch('/save', { 
                method: 'POST', 
                body: formData 
            })
            .then(response => {
                if (response.ok) {
                    showStatus('WiFi settings saved! Device is rebooting...');
                    // Start looking for the device after it connects to WiFi
                    setTimeout(startDeviceSearch, 8000); // Wait 8 seconds for reboot
                } else {
                    throw new Error('Failed to save settings');
                }
            })
            .catch(err => {
                form.style.display = 'block';
                loading.style.display = 'none';
                showStatus('Error: ' + err.message, true);
            });
        });

        // Device search functionality
        let searchAttempt = 0;
        let searchInterval = null;
        
        function startDeviceSearch() {
            searchAttempt = 0;
            showStatus('✅ WiFi settings saved! Stay here - you will be automatically redirected to the registration page...');
            
            // Start periodic search every 8 seconds
            searchInterval = setInterval(performDeviceSearch, 8000);
            
            // Also try immediately after 8 seconds
            setTimeout(performDeviceSearch, 8000);
        }
        
        function performDeviceSearch() {
            searchAttempt++;
            
            // After 3 attempts (24 seconds), just try to redirect
            if (searchAttempt >= 3) {
                clearInterval(searchInterval);
                showStatus('🚀 Device ready! Redirecting to registration page in 2 seconds...');
                setTimeout(() => {
                    window.location.href = 'http://##MDNS_HOSTNAME##.local/';
                }, 2000);
                return;
            }
            
            // Update status for early attempts
            const remainingTime = (4-searchAttempt) * 8;
            showStatus(`⏳ Device connecting to WiFi... Stay here! Auto-redirect in ${remainingTime} seconds (${searchAttempt}/3)`);
        }

        function tryMdnsAccess() {
            return new Promise((resolve) => {
                // Try to load a simple resource from the device to test connectivity
                const img = new Image();
                img.onload = () => {
                    showStatus('Device found! Redirecting to web interface...');
                    setTimeout(() => {
                        window.location.href = 'http://' + window.location.hostname + '/';
                    }, 2000);
                    resolve(true);
                };
                img.onerror = () => {
                    // Try direct redirect approach - if user is on same WiFi, it should work
                    showStatus('Trying direct connection...');
                    setTimeout(() => {
                        window.location.href = 'http://' + window.location.hostname + '/';
                    }, 1000);
                    resolve(true); // Assume it will work
                };
                
                // Try to load favicon or any small resource
                img.src = 'http://' + window.location.hostname + '/favicon.ico?' + Date.now();
                
                // Timeout after 5 seconds
                setTimeout(() => {
                    img.onload = null;
                    img.onerror = null;
                    resolve(false);
                }, 5000);
            });
        }

        function tryIpScan() {
            return new Promise(resolve => {
                const commonRanges = [
                    '192.168.1.',
                    '192.168.0.',
                    '192.168.4.',
                    '10.0.0.'
                ];
                
                let found = false;
                let completed = 0;
                const totalChecks = commonRanges.length * 50; // Check 50 IPs per range
                
                commonRanges.forEach(range => {
                    for (let i = 100; i < 150; i++) {
                        const ip = range + i;
                        
                        fetch(`http://${ip}/`, { 
                            method: 'HEAD', 
                            mode: 'no-cors',
                            timeout: 1000 
                        })
                        .then(() => {
                            if (!found) {
                                found = true;
                                showStatus(`Device found at ${ip}! Redirecting...`);
                                setTimeout(() => {
                                    window.location.href = `http://${ip}/`;
                                }, 2000);
                                resolve(true);
                            }
                        })
                        .catch(() => {})
                        .finally(() => {
                            completed++;
                            if (completed === totalChecks && !found) {
                                resolve(false);
                            }
                        });
                    }
                });
            });
        }

        // Auto-scan networks on page load
        document.addEventListener('DOMContentLoaded', function() {
            scanNetworks();
        });
    </script>
</body>
</html>
)rawliteral";
