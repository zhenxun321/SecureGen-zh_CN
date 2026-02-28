#ifndef PAGE_TEST_ENCRYPTION_H
#define PAGE_TEST_ENCRYPTION_H

const char page_test_encryption_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 安全测试 v3.0 - 请求头混淆</title>
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1000px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        .test-section { margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }
        .success { background-color: #d4edda; border-color: #c3e6cb; color: #155724; }
        .error { background-color: #f8d7da; border-color: #f5c6cb; color: #721c24; }
        .info { background-color: #d1ecf1; border-color: #bee5eb; color: #0c5460; }
        button { padding: 10px 15px; margin: 5px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }
        button:hover { background: #0056b3; }
        pre { background: #f8f9fa; padding: 10px; border-radius: 4px; overflow-x: auto; white-space: pre-wrap; }
        .log-entry { margin: 5px 0; padding: 8px; border-left: 4px solid #007bff; background: #f8f9fa; }
        .log-secure { border-left-color: #28a745; }
        .log-error { border-left-color: #dc3545; }
        .status { display: inline-block; padding: 3px 8px; border-radius: 3px; font-size: 0.9em; }
        .status.encrypted { background: #28a745; color: white; }
        .status.plaintext { background: #ffc107; color: black; }
        .status.failed { background: #dc3545; color: white; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔐 ESP32 安全集成测试 v3.0</h1>
        <p>用于验证 SecureLayerManager、方法隧道、URL 混淆和流量填充的综合测试页面。</p>
        <div style="background: #e3f2fd; padding: 10px; border-radius: 5px; margin: 10px 0; border-left: 4px solid #2196f3;">
            <strong>🏷️ NEW: HTTP Headers Obfuscation v3.0</strong> - Маскировка заголовков, внедрение ложных заголовков, и скрытие данных в User-Agent
        </div>

        <div class="test-section info">
            <h3>📋 Тестовые эндпоинты:</h3>
            <ul>
                <li><strong>/api/passwords/get</strong> - Получение расшифрованных паролей (КРИТИЧЕСКИ ЧУВСТВИТЕЛЬНЫЙ)</li>
                <li><strong>/api/keys</strong> - Получение TOTP кодов и секретов (ВЫСОКО ЧУВСТВИТЕЛЬНЫЙ)</li>
            </ul>
        </div>

        <div class="test-section">
            <h3>🔑 安全连接初始化</h3>
            <button onclick="initializeSecureClient()">初始化安全客户端</button>
            <button onclick="testKeyExchange()">测试密钥交换</button>
            <div id="connectionStatus">状态：未初始化</div>
        </div>

        <div class="test-section">
            <h3>🚇 方法隧道控制</h3>
            <button onclick="enableMethodTunneling()">启用方法隧道</button>
            <button onclick="disableMethodTunneling()">禁用方法隧道</button>
            <button onclick="showTunnelingStats()">查看隧道统计</button>
            <div id="tunnelingStatus">状态：方法隧道已禁用</div>
        </div>

        <div class="test-section">
            <h3>🏷️ HTTP 请求头混淆控制</h3>
            <button onclick="enableHeaderObfuscation()">启用请求头混淆</button>
            <button onclick="disableHeaderObfuscation()">禁用请求头混淆</button>
            <button onclick="showHeaderObfuscationStats()">查看请求头统计</button>
            <button onclick="demonstrateHeaderMasking()">演示请求头伪装</button>
            <div id="headerObfuscationStatus">状态：请求头混淆已禁用</div>
        </div>

        <div class="test-section">
            <h3>🧪 API 加密测试</h3>
            <button onclick="testPasswordsAPI()">测试 /api/passwords/get（加密）</button>
            <button onclick="testKeysAPI()">测试 /api/keys（加密）</button>
            <button onclick="testPlaintextMode()">测试明文模式（回退）</button>
        </div>

        <div class="test-section">
            <h3>🚇 方法隧道测试</h3>
            <button onclick="testTunneledPasswordsAPI()">测试隧道密码 API</button>
            <button onclick="testTunneledKeysAPI()">测试隧道密钥 API</button>
            <button onclick="testMixedRequests()">测试标准/隧道混合请求</button>
        </div>

        <div class="test-section">
            <h3>📊 测试结果</h3>
            <div id="testResults"></div>
        </div>

        <div class="test-section">
            <h3>📝 调试日志</h3>
            <button onclick="clearLogs()">Clear Logs</button>
            <div id="debugLogs"></div>
        </div>
    </div>

    <script>
        // SecureClient implementation with Method Tunneling
        class TestSecureClient {
            constructor() {
                this.sessionId = null;
                this.isSecureReady = false;
                this.serverPubKey = null;
                this.logs = [];
                this.urlMappings = new Map(); // Cache for obfuscated URLs
                this.methodTunnelingEnabled = false;
                this.tunnelingStats = { totalRequests: 0, tunneledRequests: 0 };
                
                // Header Obfuscation
                this.headerObfuscationEnabled = false;
                this.headerObfuscationStats = {
                    totalObfuscated: 0,
                    headersMapped: 0,
                    fakeHeadersInjected: 0,
                    payloadEmbedded: 0,
                    lastObfuscationTime: null
                };
            }

            generateSessionId() {
                return Array.from(crypto.getRandomValues(new Uint8Array(16)))
                    .map(b => b.toString(16).padStart(2, '0')).join('');
            }

            log(message, type = 'info') {
                const timestamp = new Date().toLocaleTimeString();
                const logEntry = `[${timestamp}] ${message}`;
                this.logs.push({ message: logEntry, type });
                this.updateLogsDisplay();
                console.log(logEntry);
            }

            // Header Obfuscation Functions
            processHeadersWithObfuscation(headers, embeddedData = {}) {
                if (!this.headerObfuscationEnabled) {
                    return headers;
                }

                this.headerObfuscationStats.totalObfuscated++;
                this.headerObfuscationStats.lastObfuscationTime = Date.now();

                let obfuscatedHeaders = { ...headers };
                let headersMappedCount = 0;

                // A) Header Mapping
                if (obfuscatedHeaders['X-Client-ID']) {
                    obfuscatedHeaders['X-Req-UUID'] = obfuscatedHeaders['X-Client-ID'];
                    delete obfuscatedHeaders['X-Client-ID'];
                    headersMappedCount++;
                }
                if (obfuscatedHeaders['X-Secure-Request']) {
                    obfuscatedHeaders['X-Security-Level'] = obfuscatedHeaders['X-Secure-Request'];
                    delete obfuscatedHeaders['X-Secure-Request'];
                    headersMappedCount++;
                }

                // B) Fake Headers Injection
                const fakeHeaders = {
                    'X-Browser-Engine': 'Mozilla/5.0 (compatible; MSIE 10.0)',
                    'X-Request-Time': Date.now().toString(),
                    'X-Client-Version': '2.4.1-stable',
                    'X-Feature-Flags': 'analytics,tracking,ads',
                    'X-Session-State': 'active'
                };
                Object.assign(obfuscatedHeaders, fakeHeaders);

                // C) Header Payload Embedding
                if (embeddedData && Object.keys(embeddedData).length > 0) {
                    const encodedData = btoa(JSON.stringify(embeddedData));
                    const baseUserAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36';
                    obfuscatedHeaders['User-Agent'] = `${baseUserAgent} (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36 EdgeInsight/${encodedData}`;
                    this.headerObfuscationStats.payloadEmbedded++;
                }

                this.headerObfuscationStats.headersMapped += headersMappedCount;
                this.headerObfuscationStats.fakeHeadersInjected += Object.keys(fakeHeaders).length;

                this.log(`🏷️ Headers obfuscated: ${headersMappedCount} mapped, ${Object.keys(fakeHeaders).length} fake added`);
                return obfuscatedHeaders;
            }

            // Method Tunneling Functions
            enableMethodTunneling() {
                this.methodTunnelingEnabled = true;
                this.log('🚇 Method Tunneling ENABLED - HTTP methods will be encrypted and tunneled', 'success');
            }

            disableMethodTunneling() {
                this.methodTunnelingEnabled = false;
                this.log('🚇 Method Tunneling DISABLED - Using standard HTTP methods', 'info');
            }

            // XOR fallback encryption for method header (same as ESP32 implementation)
            xorEncrypt(data, key) {
                let result = '';
                for (let i = 0; i < data.length; i++) {
                    const charCode = data.charCodeAt(i) ^ key.charCodeAt(i % key.length);
                    // Convert to HEX (same as server implementation)
                    result += charCode.toString(16).padStart(2, '0');
                }
                return result; // HEX encoded string (matches server)
            }

            // Encrypt method for X-Real-Method header
            encryptMethod(method) {
                // Generate the same key as server: "MT_ESP32_" + clientId + "_METHOD_KEY"
                const clientId = this.sessionId || 'UNKNOWN';
                const encryptionKey = 'MT_ESP32_' + clientId + '_METHOD_KEY';
                
                // Limit key length for consistency (max 32 chars like server)
                const limitedKey = encryptionKey.substring(0, 32);
                
                const encryptedMethod = this.xorEncrypt(method, limitedKey);
                this.log(`🔐 Method '${method}' encrypted with matching server key`, 'info');
                return encryptedMethod;
            }

            // Check if endpoint should be tunneled
            shouldTunnelEndpoint(endpoint) {
                const tunneledEndpoints = [
                    '/api/passwords',     // All passwords list
                    '/api/passwords/get',
                    '/api/keys',
                    '/api/passwords/add',
                    '/api/passwords/delete',
                    '/api/passwords/update',
                    '/api/passwords/reorder',
                    '/api/passwords/export',
                    '/api/passwords/import'
                ];
                return tunneledEndpoints.includes(endpoint);
            }

            updateLogsDisplay() {
                const logsDiv = document.getElementById('debugLogs');
                logsDiv.innerHTML = this.logs.map(log => 
                    `<div class="log-entry ${log.type === 'success' ? 'log-secure' : log.type === 'error' ? 'log-error' : ''}">${log.message}</div>`
                ).join('');
                logsDiv.scrollTop = logsDiv.scrollHeight;
            }

            async fetchURLMappings() {
                try {
                    this.log('🔗 Fetching obfuscated URL mappings...');
                    const controller = new AbortController();
                    const timeoutId = setTimeout(() => controller.abort(), 2000); // 2 second timeout
                    
                    const response = await fetch('/api/obfuscation/mappings', {
                        signal: controller.signal
                    });
                    clearTimeout(timeoutId);
                    
                    if (response.ok) {
                        const data = await response.json();
                        this.urlMappings.clear();
                        if (data.mappings && Object.keys(data.mappings).length > 0) {
                            for (const [original, obfuscated] of Object.entries(data.mappings)) {
                                this.urlMappings.set(original, obfuscated);
                            }
                            this.log(`🔗 Loaded ${Object.keys(data.mappings).length} URL mappings`, 'success');
                            return true;
                        } else {
                            this.log(`📝 No URL obfuscation mappings available, using original URLs`);
                            return false;
                        }
                    } else {
                        this.log(`📝 URL obfuscation not enabled (${response.status}), using original URLs`);
                        return false;
                    }
                } catch (error) {
                    if (error.name === 'AbortError') {
                        this.log(`📝 URL obfuscation service timeout, using original URLs`);
                    } else {
                        this.log(`📝 URL obfuscation not available, using original URLs`);
                    }
                    return false;
                }
            }

            getURL(originalURL) {
                const obfuscated = this.urlMappings.get(originalURL);
                if (obfuscated && obfuscated !== originalURL) {
                    this.log(`🔗 Using obfuscated URL: ${originalURL} → ${obfuscated}`);
                    return obfuscated;
                }
                this.log(`📝 Using original URL: ${originalURL}`);
                return originalURL;
            }

            async testHello() {
                try {
                    this.log('🔐 Testing /api/secure/hello endpoint...');
                    const response = await fetch('/api/secure/hello');
                    const data = await response.json();
                    
                    if (response.ok) {
                        this.log(`✅ Hello response: ${JSON.stringify(data)}`, 'success');
                        return true;
                    } else {
                        this.log(`❌ Hello failed: ${response.status}`, 'error');
                        return false;
                    }
                } catch (error) {
                    this.log(`❌ Hello error: ${error.message}`, 'error');
                    return false;
                }
            }

            async performKeyExchange() {
                try {
                    this.sessionId = this.generateSessionId();
                    this.log(`🔐 Starting REAL key exchange with ESP32...`);
                    this.log(`🔐 Session ID: ${this.sessionId.substring(0,8)}...`);

                    // Real key exchange with ESP32 SecureLayerManager
                    const keyExchangeData = {
                        client_id: this.sessionId,
                        client_public_key: "04700c48f77f56584c5cc632ca65640db91b6bacce3a4df6b42ce7cc838833d287db71e509e3fd9b060ddb20ba5c51dcc5948d46fbf640dfe0441782cab85fa4ac"
                    };
                    
                    this.log(`🔐 Sending key exchange request to ESP32...`);
                    this.log(`🔐 Payload: ${JSON.stringify(keyExchangeData)}`);

                    const keyExchangeURL = this.getURL('/api/secure/keyexchange');
                    const response = await fetch(keyExchangeURL, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                            'X-Client-ID': this.sessionId
                        },
                        body: JSON.stringify(keyExchangeData)
                    });

                    this.log(`🔐 ESP32 response status: ${response.status}`);
                    
                    if (response.ok) {
                        const data = await response.json();
                        this.log(`✅ ESP32 Key exchange successful!`, 'success');
                        this.log(`🔐 ESP32 response: ${JSON.stringify(data)}`, 'success');
                        this.isSecureReady = true;
                        this.serverPubKey = data.pubkey;
                        this.log(`🔐 Secure session established with ESP32!`, 'success');
                        return true;
                    } else {
                        const errorText = await response.text();
                        this.log(`❌ ESP32 Key exchange failed: ${response.status}`, 'error');
                        this.log(`❌ ESP32 Error response: ${errorText}`, 'error');
                        return false;
                    }
                } catch (error) {
                    this.log(`❌ Key exchange network error: ${error.message}`, 'error');
                    return false;
                }
            }

            async testSecureAPIFormData(endpoint, method = 'GET', formData = null) {
                try {
                    const headers = {};

                    if (this.isSecureReady && this.sessionId) {
                        headers['X-Client-ID'] = this.sessionId;
                        headers['X-Secure-Request'] = 'true';
                        this.log(`🔐 Testing ${endpoint} with SECURE encryption (form data)...`);
                    } else {
                        this.log(`📝 Testing ${endpoint} in PLAINTEXT mode (fallback, form data)...`);
                    }

                    let body = null;
                    if (formData) {
                        body = new URLSearchParams();
                        for (const [key, value] of Object.entries(formData)) {
                            body.append(key, value);
                        }
                    }

                    const actualURL = this.getURL(endpoint);
                    const response = await fetch(actualURL, {
                        method,
                        headers,
                        body
                    });

                    const responseText = await response.text();
                    
                    if (response.ok) {
                        try {
                            const data = JSON.parse(responseText);
                            
                            // Check if response is encrypted (ESP32 format: {"type":"secure","data":"...","iv":"...","tag":"..."})
                            const isEncrypted = data.type === "secure" && data.data && data.iv && data.tag;
                            if (isEncrypted) {
                                this.log(`🔐 Encrypted data length: ${data.data.length} hex chars`);
                                return { success: true, data, encrypted: true };
                            } else {
                                this.log(`📝 ${endpoint} - Plaintext response received`);
                                return { success: true, data, encrypted: false };
                            }
                        } catch (parseError) {
                            this.log(`❌ Failed to parse JSON response: ${parseError.message}`, 'error');
                            return { success: false, error: 'JSON parse error', encrypted: false };
                        }
                    } else {
                        this.log(`❌ ${endpoint} failed: ${response.status} - ${responseText}`, 'error');
                        return { success: false, error: `HTTP ${response.status}`, encrypted: false };
                    }
                } catch (error) {
                    this.log(`❌ ${endpoint} network error: ${error.message}`, 'error');
                    return { success: false, error: error.message, encrypted: false };
                }
            }

            async testSecureAPI(endpoint, method = 'GET', body = null) {
                try {
                    this.tunnelingStats.totalRequests++;
                    
                    let headers = {
                        'Content-Type': 'application/json'
                    };

                    // Check if method tunneling should be used
                    const shouldTunnel = this.methodTunnelingEnabled && this.shouldTunnelEndpoint(endpoint);
                    let actualMethod = method;
                    let actualEndpoint = endpoint;

                    if (shouldTunnel) {
                        // Method Tunneling: Convert to POST with encrypted X-Real-Method
                        const encryptedMethod = this.encryptMethod(method);
                        headers['X-Real-Method'] = encryptedMethod;
                        actualMethod = 'POST';
                        actualEndpoint = '/api/tunnel'; // Use tunnel endpoint
                        
                        // Add original endpoint to body for tunneling
                        const tunnelBody = {
                            endpoint: endpoint,
                            data: body || {}
                        };
                        body = tunnelBody;
                        
                        this.tunnelingStats.tunneledRequests++;
                        this.log(`🚇 TUNNELING ${method} ${endpoint} -> POST /api/tunnel`, 'success');
                        this.log(`🔐 X-Real-Method: ${encryptedMethod.substring(0, 20)}...`, 'success');
                    }

                    // КРИТИЧНО: Всегда добавляем X-Client-ID для tunneled запросов
                    if (this.isSecureReady && this.sessionId) {
                        headers['X-Client-ID'] = this.sessionId;
                        headers['X-Secure-Request'] = 'true';
                        if (!shouldTunnel) {
                            this.log(`🔐 Testing ${endpoint} with SECURE encryption...`);
                        }
                    } else if (shouldTunnel && this.sessionId) {
                        // Fallback: добавляем clientId даже если secure session не ready для tunneled запросов
                        headers['X-Client-ID'] = this.sessionId;
                        this.log(`🚇 Adding clientId to tunneled request (fallback mode)`, 'info');
                    } else {
                        if (!shouldTunnel) {
                            this.log(`📝 Testing ${endpoint} in PLAINTEXT mode (fallback)...`);
                        }
                    }

                    // Apply Header Obfuscation
                    const embeddedData = { 
                        endpoint: endpoint, 
                        method: actualMethod, 
                        tunneled: shouldTunnel,
                        timestamp: Date.now()
                    };
                    headers = this.processHeadersWithObfuscation(headers, embeddedData);

                    const actualURL = this.getURL(actualEndpoint);
                    const response = await fetch(actualURL, {
                        method: actualMethod,
                        headers,
                        body: body ? JSON.stringify(body) : null
                    });

                    const responseText = await response.text();
                    
                    // DEBUG: Показываем RAW response от сервера
                    if (shouldTunnel) {
                        this.log(`🔍 RAW server response: ${responseText.substring(0, 150)}...`, 'debug');
                    }
                    
                    // КРИТИЧНО: Проверяем raw response на наличие зашифрованных полей ДО парсинга
                    const hasEncryptedFields = responseText.includes('"iv"') && responseText.includes('"tag"') && responseText.includes('"data"');
                    const wasEncrypted = hasEncryptedFields;
                    
                    if (response.ok) {
                        try {
                            const data = JSON.parse(responseText);
                            // Логика определения зашифрованных ответов
                            const isSecureFormat = data.type === 'secure' && data.data && data.iv && data.tag;
                            const isEncryptedData = data.data && data.iv && data.tag && !data.type;
                            
                            const isEncrypted = isSecureFormat || isEncryptedData || wasEncrypted;
                            
                            if (isEncrypted) {
                                this.log(`✅ ${endpoint} - ENCRYPTED response received!`, 'success');
                                const dataLength = data.data ? data.data.length : 'unknown';
                                this.log(`🔐 Encrypted data length: ${dataLength} chars`, 'success');
                                if (shouldTunnel) {
                                    this.log(`🚇 Method successfully tunneled and encrypted!`, 'success');
                                }
                                return { success: true, encrypted: true, tunneled: shouldTunnel, data: data, raw: responseText };
                            } else {
                                this.log(`📝 ${endpoint} - Plaintext response received`, 'info');
                                if (shouldTunnel) {
                                    this.log(`🚇 Method tunneled but response in plaintext`, 'info');
                                }
                                // Для tunneled запросов добавляем детали для отладки
                                if (shouldTunnel) {
                                    this.log(`🔍 Response format: ${JSON.stringify(data).substring(0, 100)}...`, 'debug');
                                }
                                return { success: true, encrypted: false, tunneled: shouldTunnel, data: data, raw: responseText };
                            }
                        } catch (e) {
                            this.log(`📝 ${endpoint} - Raw response (${responseText.length} chars)`, 'info');
                            return { success: true, encrypted: false, tunneled: shouldTunnel, raw: responseText };
                        }
                    } else {
                        this.log(`❌ ${endpoint} failed: ${response.status} - ${responseText}`, 'error');
                        if (shouldTunnel) {
                            this.log(`❌ Tunneled request failed!`, 'error');
                        }
                        return { success: false, tunneled: shouldTunnel, error: responseText };
                    }
                } catch (error) {
                    this.log(`❌ ${endpoint} error: ${error.message}`, 'error');
                    return { success: false, error: error.message };
                }
            }
        }

        // Global test client
        const testClient = new TestSecureClient();

        // UI Functions
        function updateConnectionStatus(message, type = 'info') {
            const statusDiv = document.getElementById('connectionStatus');
            statusDiv.innerHTML = `Status: ${message}`;
            statusDiv.className = type;
        }

        function addTestResult(testName, result) {
            const resultsDiv = document.getElementById('testResults');
            const statusClass = result.success ? (result.encrypted ? 'encrypted' : 'plaintext') : 'failed';
            let statusText = result.success ? (result.encrypted ? 'ENCRYPTED' : 'PLAINTEXT') : 'FAILED';
            
            // Add tunneling indicator
            if (result.tunneled) {
                statusText += ' + TUNNELED';
            }
            
            const resultHtml = `
                <div class="test-section">
                    <h4>${testName} ${result.tunneled ? '🚇' : ''} <span class="status ${statusClass}">${statusText}</span></h4>
                    <pre>${JSON.stringify(result, null, 2)}</pre>
                </div>
            `;
            resultsDiv.innerHTML += resultHtml;
        }

        function updateTunnelingStatus(message, type = 'info') {
            const statusDiv = document.getElementById('tunnelingStatus');
            statusDiv.innerHTML = `Status: ${message}`;
            statusDiv.className = type;
        }

        function updateHeaderObfuscationStatus(message, type = 'info') {
            const statusDiv = document.getElementById('headerObfuscationStatus');
            statusDiv.innerHTML = `Status: ${message}`;
            statusDiv.className = type;
        }

        // Header Obfuscation Control Functions
        function enableHeaderObfuscation() {
            testClient.headerObfuscationEnabled = true;
            testClient.log('🏷️ Header Obfuscation ENABLED', 'success');
            updateHeaderObfuscationStatus('Header Obfuscation Enabled', 'success');
        }

        function disableHeaderObfuscation() {
            testClient.headerObfuscationEnabled = false;
            testClient.log('🏷️ Header Obfuscation DISABLED', 'info');
            updateHeaderObfuscationStatus('Header Obfuscation Disabled', 'info');
        }

        function showHeaderObfuscationStats() {
            testClient.log('🏷️ === HEADER OBFUSCATION STATISTICS ===');
            testClient.log(`📊 Total obfuscated requests: ${testClient.headerObfuscationStats.totalObfuscated}`);
            testClient.log(`🎭 Headers mapped: ${testClient.headerObfuscationStats.headersMapped}`);
            testClient.log(`🔀 Fake headers injected: ${testClient.headerObfuscationStats.fakeHeadersInjected}`);
            testClient.log(`📦 Payloads embedded in headers: ${testClient.headerObfuscationStats.payloadEmbedded}`);
            if (testClient.headerObfuscationStats.lastObfuscationTime) {
                testClient.log(`⏰ Last obfuscation: ${new Date(testClient.headerObfuscationStats.lastObfuscationTime).toLocaleTimeString()}`);
            }
        }

        function demonstrateHeaderMasking() {
            testClient.log('🏷️ === HEADER MASKING DEMONSTRATION ===');
            
            const originalHeaders = {
                'Content-Type': 'application/json',
                'X-Client-ID': 'demo-client-12345',
                'X-Secure-Request': 'true',
                'Authorization': 'Bearer demo-token'
            };
            
            testClient.log('📤 Original headers:');
            Object.entries(originalHeaders).forEach(([key, value]) => {
                testClient.log(`   ${key}: ${value}`);
            });
            
            const embeddedData = { type: 'demo', timestamp: Date.now() };
            const obfuscatedHeaders = testClient.processHeadersWithObfuscation(originalHeaders, embeddedData);
            
            testClient.log('🎭 Obfuscated headers:');
            Object.entries(obfuscatedHeaders).forEach(([key, value]) => {
                testClient.log(`   ${key}: ${value}`);
            });
        }

        // Test Functions
        async function initializeSecureClient() {
            updateConnectionStatus('Initializing...', 'info');
            
            // First, try to load URL mappings for obfuscated endpoints
            await testClient.fetchURLMappings();
            
            const helloSuccess = await testClient.testHello();
            if (helloSuccess) {
                updateConnectionStatus('Hello endpoint OK - Ready for key exchange', 'success');
            } else {
                updateConnectionStatus('Hello endpoint failed', 'error');
            }
        }

        async function testKeyExchange() {
            updateConnectionStatus('Performing key exchange...', 'info');
            
            const keyExchangeSuccess = await testClient.performKeyExchange();
            if (keyExchangeSuccess) {
                updateConnectionStatus('Secure connection established!', 'success');
            } else {
                updateConnectionStatus('Key exchange failed', 'error');
            }
        }

        async function testPasswordsAPI() {
            testClient.log('🔐 === TESTING PASSWORDS API (CRITICAL) ===');
            
            // Тест с фиктивным индексом пароля - отправляем как form data
            const result = await testClient.testSecureAPIFormData('/api/passwords/get', 'POST', { index: '0' });
            addTestResult('Password API Test', result);
            
            if (result.success && result.encrypted) {
                testClient.log('🎉 PASSWORD DATA SUCCESSFULLY ENCRYPTED!', 'success');
            } else if (result.success) {
                testClient.log('⚠️ Password data sent in plaintext (fallback)', 'info');
            }
        }

        async function testKeysAPI() {
            testClient.log('🔐 === TESTING KEYS API (TOTP CODES) ===');
            
            const result = await testClient.testSecureAPI('/api/keys', 'GET');
            addTestResult('Keys API Test', result);
            
            if (result.success && result.encrypted) {
                testClient.log('🎉 TOTP KEYS DATA SUCCESSFULLY ENCRYPTED!', 'success');
            } else if (result.success) {
                testClient.log('⚠️ TOTP keys sent in plaintext (fallback)', 'info');
            }
        }

        async function testPlaintextMode() {
            testClient.log('📝 === TESTING PLAINTEXT MODE (NO ENCRYPTION) ===');
            
            // Временно отключаем secure mode
            const originalState = testClient.isSecureReady;
            testClient.isSecureReady = false;
            
            const result = await testClient.testSecureAPI('/api/keys', 'GET');
            addTestResult('Plaintext Mode Test', result);
            
            // Восстанавливаем состояние
            testClient.isSecureReady = originalState;
        }

        // Method Tunneling Controls
        function enableMethodTunneling() {
            testClient.enableMethodTunneling();
            updateTunnelingStatus('Method Tunneling ENABLED', 'success');
        }

        function disableMethodTunneling() {
            testClient.disableMethodTunneling();
            updateTunnelingStatus('Method Tunneling DISABLED', 'info');
        }

        function showTunnelingStats() {
            const stats = testClient.tunnelingStats;
            const percentage = stats.totalRequests > 0 ? 
                ((stats.tunneledRequests / stats.totalRequests) * 100).toFixed(1) : 0;
            
            testClient.log(`📊 TUNNELING STATISTICS:`, 'info');
            testClient.log(`📊 Total Requests: ${stats.totalRequests}`, 'info');
            testClient.log(`📊 Tunneled Requests: ${stats.tunneledRequests}`, 'info');
            testClient.log(`📊 Tunneling Rate: ${percentage}%`, 'info');
        }

        // Method Tunneling Tests
        async function testTunneledPasswordsAPI() {
            testClient.log('🚇 === TESTING TUNNELED PASSWORDS API ===');
            
            const originalState = testClient.methodTunnelingEnabled;
            testClient.enableMethodTunneling();
            
            const result = await testClient.testSecureAPIFormData('/api/passwords/get', 'POST', 
                new URLSearchParams({index: '0'}));
            addTestResult('Tunneled Password API Test', result);
            
            if (result.success && result.tunneled) {
                testClient.log('🎉 PASSWORD API SUCCESSFULLY TUNNELED!', 'success');
            }
            
            testClient.methodTunnelingEnabled = originalState;
        }

        async function testTunneledKeysAPI() {
            testClient.log('🚇 === TESTING TUNNELED KEYS API ===');
            
            const originalState = testClient.methodTunnelingEnabled;
            testClient.enableMethodTunneling();
            
            const result = await testClient.testSecureAPI('/api/keys', 'GET');
            addTestResult('Tunneled Keys API Test', result);
            
            if (result.success && result.tunneled) {
                testClient.log('🎉 KEYS API SUCCESSFULLY TUNNELED!', 'success');
            }
            
            testClient.methodTunnelingEnabled = originalState;
        }

        async function testMixedRequests() {
            testClient.log('🚇 === TESTING MIXED STANDARD/TUNNELED REQUESTS ===');
            
            // Test 1: Standard request
            testClient.disableMethodTunneling();
            const standardResult = await testClient.testSecureAPI('/api/keys', 'GET');
            addTestResult('Mixed Test - Standard Request', standardResult);
            
            // Test 2: Tunneled request
            testClient.enableMethodTunneling();
            const tunneledResult = await testClient.testSecureAPI('/api/keys', 'GET');
            addTestResult('Mixed Test - Tunneled Request', tunneledResult);
            
            // Compare results
            if (standardResult.success && tunneledResult.success) {
                testClient.log('🎉 MIXED REQUESTS TEST PASSED!', 'success');
                testClient.log(`📊 Standard: ${standardResult.encrypted ? 'ENCRYPTED' : 'PLAINTEXT'}`, 'info');
                testClient.log(`📊 Tunneled: ${tunneledResult.encrypted ? 'ENCRYPTED' : 'PLAINTEXT'} + TUNNELED`, 'info');
            }
        }

        function clearLogs() {
            testClient.logs = [];
            testClient.updateLogsDisplay();
            document.getElementById('testResults').innerHTML = '';
            testClient.tunnelingStats = { totalRequests: 0, tunneledRequests: 0 };
        }

        // Initialize on load
        window.onload = function() {
            testClient.log('🚀 ESP32 Security Integration Test v3.0 initialized');
            testClient.log('📝 Ready to test secure communication with ESP32 device');
            testClient.log('🚇 Method Tunneling feature added for traffic analysis resistance');
            testClient.log('🏷️ Header Obfuscation v3.0 feature added for enhanced security');
            updateConnectionStatus('Not initialized', 'info');
            updateTunnelingStatus('Method Tunneling Disabled', 'info');
            updateHeaderObfuscationStatus('Header Obfuscation Disabled', 'info');
        };
    </script>
</body>
</html>
)rawliteral";

#endif
