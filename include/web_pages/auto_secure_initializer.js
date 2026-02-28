/**
 * ESP32 Protected Handshake Auto-Initializer
 * Автоматически инициализирует защищенную сессию с device-specific key wrapping
 * 
 * Функциональность:
 * - Автоматическая генерация ECDH ключей
 * - Шифрование client public key с device static key
 * - Обмен защищенными ключами с сервером
 * - Activity tracking для поддержания сессии
 * - Browser fingerprinting для уникальности
 */

(function() {
    'use strict';
    
    // Global state
    let secureSession = {
        initialized: false,
        clientId: null,
        keyPair: null,
        sharedSecret: null,
        sessionKey: null,
        deviceKey: null,
        lastActivity: Date.now()
    };
    
    // Configuration
    const CONFIG = {
        ACTIVITY_INTERVAL: 30000, // 30 seconds
        HANDSHAKE_TIMEOUT: 10000, // 10 seconds
        MAX_RETRY_ATTEMPTS: 3,
        DEBUG: true
    };
    
    /**
     * Logging utility
     */
    function log(level, message, data = null) {
        if (!CONFIG.DEBUG && level === 'debug') return;
        
        const timestamp = new Date().toISOString();
        const prefix = `[ESP32-SecureInit] ${timestamp} [${level.toUpperCase()}]`;
        
        if (data) {
            console[level](prefix, message, data);
        } else {
            console[level](prefix, message);
        }
    }
    
    /**
     * Generate device static key from browser fingerprint
     */
    async function generateDeviceStaticKey() {
        log('debug', 'Generating device static key from browser fingerprint');
        
        // Собираем browser fingerprint
        const fingerprint = {
            userAgent: navigator.userAgent,
            language: navigator.language,
            platform: navigator.platform,
            timezone: Intl.DateTimeFormat().resolvedOptions().timeZone,
            screen: `${screen.width}x${screen.height}x${screen.colorDepth}`,
            canvas: getCanvasFingerprint(),
            webgl: getWebGLFingerprint(),
            fonts: await getFontFingerprint(),
            timestamp: Math.floor(Date.now() / (1000 * 60 * 60 * 24)) // Daily rotation
        };
        
        // Конвертируем в строку для хеширования
        const fingerprintString = JSON.stringify(fingerprint);
        log('debug', 'Browser fingerprint generated', fingerprint);
        
        // Хешируем fingerprint с помощью SHA-256
        const encoder = new TextEncoder();
        const data = encoder.encode(fingerprintString);
        const hashBuffer = await crypto.subtle.digest('SHA-256', data);
        
        // Конвертируем в Base64
        const hashArray = new Uint8Array(hashBuffer);
        const deviceKey = btoa(String.fromCharCode(...hashArray));
        
        log('debug', 'Device static key generated', { length: deviceKey.length });
        return deviceKey;
    }
    
    /**
     * Canvas fingerprinting
     */
    function getCanvasFingerprint() {
        try {
            const canvas = document.createElement('canvas');
            const ctx = canvas.getContext('2d');
            ctx.textBaseline = 'top';
            ctx.font = '14px Arial';
            ctx.fillText('ESP32 TOTP Fingerprint 🔐', 2, 2);
            return canvas.toDataURL();
        } catch (e) {
            return 'canvas-unavailable';
        }
    }
    
    /**
     * WebGL fingerprinting
     */
    function getWebGLFingerprint() {
        try {
            const canvas = document.createElement('canvas');
            const gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
            if (!gl) return 'webgl-unavailable';
            
            const debugInfo = gl.getExtension('WEBGL_debug_renderer_info');
            if (!debugInfo) return 'debug-info-unavailable';
            
            return {
                vendor: gl.getParameter(debugInfo.UNMASKED_VENDOR_WEBGL),
                renderer: gl.getParameter(debugInfo.UNMASKED_RENDERER_WEBGL)
            };
        } catch (e) {
            return 'webgl-error';
        }
    }
    
    /**
     * Font detection fingerprinting
     */
    async function getFontFingerprint() {
        if (!document.fonts || !document.fonts.check) {
            return 'fonts-api-unavailable';
        }
        
        const testFonts = ['Arial', 'Times New Roman', 'Courier New', 'Helvetica', 'Georgia'];
        const availableFonts = [];
        
        for (const font of testFonts) {
            if (document.fonts.check(`12px "${font}"`)) {
                availableFonts.push(font);
            }
        }
        
        return availableFonts;
    }
    
    /**
     * Generate ECDH key pair
     */
    async function generateECDHKeyPair() {
        log('debug', '正在生成 ECDH 密钥对 (P-256)');
        
        try {
            const keyPair = await crypto.subtle.generateKey(
                {
                    name: 'ECDH',
                    namedCurve: 'P-256'
                },
                true, // extractable
                ['deriveKey', 'deriveBits']
            );
            
            log('debug', 'ECDH 密钥对生成成功');
            return keyPair;
        } catch (error) {
            log('error', '生成 ECDH 密钥对失败', error);
            throw error;
        }
    }
    
    /**
     * Export public key to hex format
     */
    async function exportPublicKeyToHex(publicKey) {
        try {
            const exported = await crypto.subtle.exportKey('raw', publicKey);
            const uint8Array = new Uint8Array(exported);
            return Array.from(uint8Array).map(b => b.toString(16).padStart(2, '0')).join('');
        } catch (error) {
            log('error', '导出公钥失败', error);
            throw error;
        }
    }
    
    /**
     * Import server public key from hex
     */
    async function importServerPublicKey(hexKey) {
        try {
            const keyBytes = new Uint8Array(hexKey.match(/.{1,2}/g).map(byte => parseInt(byte, 16)));
            
            const publicKey = await crypto.subtle.importKey(
                'raw',
                keyBytes,
                {
                    name: 'ECDH',
                    namedCurve: 'P-256'
                },
                false,
                []
            );
            
            return publicKey;
        } catch (error) {
            log('error', '导入服务器公钥失败', error);
            throw error;
        }
    }
    
    /**
     * Derive shared secret using ECDH
     */
    async function deriveSharedSecret(privateKey, serverPublicKey) {
        try {
            const sharedSecret = await crypto.subtle.deriveBits(
                {
                    name: 'ECDH',
                    public: serverPublicKey
                },
                privateKey,
                256 // 32 bytes
            );
            
            return new Uint8Array(sharedSecret);
        } catch (error) {
            log('error', '派生共享密钥失败', error);
            throw error;
        }
    }
    
    /**
     * Encrypt data with password-based encryption (simplified AES-GCM)
     */
    async function encryptWithPassword(plaintext, password) {
        try {
            log('debug', '正在使用设备密钥加密数据');
            
            // Derive key from password using PBKDF2
            const encoder = new TextEncoder();
            const passwordBuffer = encoder.encode(password);
            
            const baseKey = await crypto.subtle.importKey(
                'raw',
                passwordBuffer,
                'PBKDF2',
                false,
                ['deriveKey']
            );
            
            const salt = crypto.getRandomValues(new Uint8Array(16));
            const iv = crypto.getRandomValues(new Uint8Array(12));
            
            const derivedKey = await crypto.subtle.deriveKey(
                {
                    name: 'PBKDF2',
                    salt: salt,
                    iterations: 10000,
                    hash: 'SHA-256'
                },
                baseKey,
                {
                    name: 'AES-GCM',
                    length: 256
                },
                false,
                ['encrypt']
            );
            
            const plaintextBuffer = encoder.encode(plaintext);
            const encrypted = await crypto.subtle.encrypt(
                {
                    name: 'AES-GCM',
                    iv: iv
                },
                derivedKey,
                plaintextBuffer
            );
            
            // Combine salt + iv + encrypted data
            const result = new Uint8Array(salt.length + iv.length + encrypted.byteLength);
            result.set(salt, 0);
            result.set(iv, salt.length);
            result.set(new Uint8Array(encrypted), salt.length + iv.length);
            
            // Return as Base64
            return btoa(String.fromCharCode(...result));
        } catch (error) {
            log('error', '加密失败', error);
            throw error;
        }
    }
    
    /**
     * Decrypt server response with password
     */
    async function decryptWithPassword(encryptedData, password) {
        try {
            log('debug', '正在使用设备密钥解密服务器响应');
            
            // Decode from Base64
            const data = new Uint8Array(atob(encryptedData).split('').map(c => c.charCodeAt(0)));
            
            // Extract components
            const salt = data.slice(0, 16);
            const iv = data.slice(16, 28);
            const encrypted = data.slice(28);
            
            // Derive key
            const encoder = new TextEncoder();
            const passwordBuffer = encoder.encode(password);
            
            const baseKey = await crypto.subtle.importKey(
                'raw',
                passwordBuffer,
                'PBKDF2',
                false,
                ['deriveKey']
            );
            
            const derivedKey = await crypto.subtle.deriveKey(
                {
                    name: 'PBKDF2',
                    salt: salt,
                    iterations: 10000,
                    hash: 'SHA-256'
                },
                baseKey,
                {
                    name: 'AES-GCM',
                    length: 256
                },
                false,
                ['decrypt']
            );
            
            // Decrypt
            const decrypted = await crypto.subtle.decrypt(
                {
                    name: 'AES-GCM',
                    iv: iv
                },
                derivedKey,
                encrypted
            );
            
            const decoder = new TextDecoder();
            return decoder.decode(decrypted);
        } catch (error) {
            log('error', '解密失败', error);
            throw error;
        }
    }
    
    /**
     * Perform protected handshake with server
     */
    async function performProtectedHandshake() {
        log('info', 'Starting protected handshake');
        
        try {
            // 1. Generate device static key
            secureSession.deviceKey = await generateDeviceStaticKey();
            
            // 2. Generate ECDH key pair
            secureSession.keyPair = await generateECDHKeyPair();
            
            // 3. Export client public key
            const clientPubKeyHex = await exportPublicKeyToHex(secureSession.keyPair.publicKey);
            log('debug', 'Client public key exported', { length: clientPubKeyHex.length });
            
            // 4. Encrypt client public key with device key
            const encryptedClientKey = await encryptWithPassword(clientPubKeyHex, secureSession.deviceKey);
            log('debug', 'Client key encrypted with device key');
            
            // 5. Send protected handshake request
            const response = await fetch('/secure/protected-handshake', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'X-Client-ID': secureSession.clientId
                },
                body: JSON.stringify({
                    client_id: secureSession.clientId,
                    encrypted_pubkey: encryptedClientKey
                })
            });
            
            if (!response.ok) {
                throw new Error(`Handshake failed: ${response.status} ${response.statusText}`);
            }
            
            const result = await response.json();
            
            if (result.status !== 'success') {
                throw new Error(`Handshake error: ${result.message || 'Unknown error'}`);
            }
            
            // 6. Decrypt server public key
            const serverPubKeyHex = await decryptWithPassword(result.encrypted_pubkey, secureSession.deviceKey);
            log('debug', 'Server public key decrypted');
            
            // 7. Import server public key and derive shared secret
            const serverPublicKey = await importServerPublicKey(serverPubKeyHex);
            secureSession.sharedSecret = await deriveSharedSecret(secureSession.keyPair.privateKey, serverPublicKey);
            
            log('info', 'Protected handshake completed successfully');
            secureSession.initialized = true;
            secureSession.lastActivity = Date.now();
            
            return true;
        } catch (error) {
            log('error', 'Protected handshake failed', error);
            return false;
        }
    }
    
    /**
     * Initialize secure session
     */
    async function initializeSecureSession() {
        log('info', 'Initializing ESP32 protected secure session');
        
        // Get client ID from injected global
        if (window.ESP32_CLIENT_ID) {
            secureSession.clientId = window.ESP32_CLIENT_ID;
            log('debug', 'Client ID retrieved from injection', { clientId: secureSession.clientId.substring(0, 8) + '...' });
        } else {
            log('error', 'Client ID not found in global scope');
            return false;
        }
        
        // Perform protected handshake
        let attempts = 0;
        while (attempts < CONFIG.MAX_RETRY_ATTEMPTS) {
            attempts++;
            log('debug', `握手尝试 ${attempts}/${CONFIG.MAX_RETRY_ATTEMPTS}`);
            
            if (await performProtectedHandshake()) {
                startActivityTracking();
                log('info', '🔐 受保护会话初始化成功');
                return true;
            }
            
            if (attempts < CONFIG.MAX_RETRY_ATTEMPTS) {
                await new Promise(resolve => setTimeout(resolve, 1000 * attempts));
            }
        }
        
        log('error', '多次尝试后仍无法初始化受保护会话');
        return false;
    }
    
    /**
     * Start activity tracking
     */
    function startActivityTracking() {
        log('debug', '开始活动跟踪');
        
        // Track user interactions
        const events = ['click', 'keydown', 'mousemove', 'scroll', 'focus'];
        events.forEach(event => {
            document.addEventListener(event, updateActivity, { passive: true });
        });
        
        // Periodic activity ping
        setInterval(() => {
            if (secureSession.initialized) {
                sendActivityPing();
            }
        }, CONFIG.ACTIVITY_INTERVAL);
    }
    
    /**
     * Update activity timestamp
     */
    function updateActivity() {
        secureSession.lastActivity = Date.now();
    }
    
    /**
     * Send activity ping to server
     */
    async function sendActivityPing() {
        try {
            await fetch('/api/activity', {
                method: 'POST',
                headers: {
                    'X-Client-ID': secureSession.clientId,
                    'X-Secure-Session': secureSession.initialized ? 'true' : 'false'
                }
            });
            
            log('debug', 'Activity ping sent');
        } catch (error) {
            log('debug', 'Activity ping failed', error);
        }
    }
    
    /**
     * Auto-initialization on DOM ready
     */
    function autoInitialize() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', initializeSecureSession);
        } else {
            // DOM already loaded
            setTimeout(initializeSecureSession, 100);
        }
    }
    
    // Expose to global for debugging
    window.ESP32SecureSession = {
        getState: () => ({ ...secureSession, sharedSecret: '[HIDDEN]', deviceKey: '[HIDDEN]' }),
        reinitialize: initializeSecureSession,
        isInitialized: () => secureSession.initialized
    };
    
    log('info', 'ESP32 Protected Handshake Auto-Initializer loaded');
    
    // Check if auto-init is enabled
    if (window.ESP32_SECURE_INIT) {
        autoInitialize();
    }
    
})();
