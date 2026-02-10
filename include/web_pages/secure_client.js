/**
 * SecureClient - JavaScript клиент для end-to-end шифрования
 * Интегрируется с SecureLayerManager ESP32 для защиты трафика
 */
class SecureClient {
    constructor() {
        this.keyPair = null;
        this.sharedKey = null;
        this.messageCounter = 0;
        this.sessionId = null;
        this.isEncrypted = false;
        this.debugMode = false;
        
        this.init();
    }
    
    async init() {
        try {
            await this.generateKeyPair();
            await this.initiateKeyExchange();
            this.log("SecureClient initialized", "info");
        } catch (error) {
            this.log("SecureClient initialization failed: " + error.message, "error");
        }
    }
    
    async generateKeyPair() {
        try {
            this.keyPair = await crypto.subtle.generateKey(
                { name: "ECDH", namedCurve: "P-256" },
                true,
                ["deriveKey"]
            );
            this.log("ECDH key pair generated", "debug");
        } catch (error) {
            throw new Error("Key generation failed: " + error.message);
        }
    }
    
    async initiateKeyExchange() {
        try {
            // Generate session ID
            this.sessionId = this.generateSessionId();
            
            // Export our public key
            const publicKey = await crypto.subtle.exportKey("raw", this.keyPair.publicKey);
            const publicKeyHex = Array.from(new Uint8Array(publicKey))
                .map(b => b.toString(16).padStart(2, '0')).join('');
            
            // Send key exchange request
            const response = await fetch('/api/secure/keyexchange', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                    'X-Session-ID': this.sessionId
                },
                body: JSON.stringify({
                    type: 'keyexchange',
                    pubkey: publicKeyHex,
                    session_id: this.sessionId
                })
            });
            
            if (!response.ok) {
                throw new Error("Key exchange request failed: " + response.status);
            }
            
            const data = await response.json();
            if (data.status === 'success') {
                await this.completeKeyExchange(data.pubkey);
                this.log("Key exchange completed successfully", "success");
            } else {
                throw new Error("Key exchange failed: " + data.message);
            }
            
        } catch (error) {
            throw new Error("Key exchange initiation failed: " + error.message);
        }
    }
    
    async completeKeyExchange(serverPubKeyHex) {
        try {
            // Import server's public key
            const serverKeyData = new Uint8Array(
                serverPubKeyHex.match(/.{2}/g).map(byte => parseInt(byte, 16))
            );
            
            const serverPublicKey = await crypto.subtle.importKey(
                "raw", serverKeyData,
                { name: "ECDH", namedCurve: "P-256" },
                false, []
            );
            
            // Derive shared key
            this.sharedKey = await crypto.subtle.deriveKey(
                { name: "ECDH", public: serverPublicKey },
                this.keyPair.privateKey,
                { name: "AES-GCM", length: 256 },
                false,
                ["encrypt", "decrypt"]
            );
            
            this.isEncrypted = true;
            this.log("End-to-end encryption established", "success");
            
            // Trigger secure ready event
            if (typeof window.onSecureReady === 'function') {
                window.onSecureReady();
            }
            
        } catch (error) {
            throw new Error("Key exchange completion failed: " + error.message);
        }
    }
    
    async makeSecureRequest(url, options = {}) {
        if (!this.isEncrypted) {
            // Fallback to regular request if encryption not ready
            this.log("Encryption not ready, falling back to regular request", "warning");
            return await fetch(url, options);
        }
        
        try {
            // Encrypt request body if present
            let encryptedBody = null;
            if (options.body) {
                const encrypted = await this.encrypt(options.body);
                encryptedBody = JSON.stringify(encrypted);
            }
            
            // Prepare secure request
            const secureOptions = {
                ...options,
                headers: {
                    ...options.headers,
                    'Content-Type': 'application/json',
                    'X-Session-ID': this.sessionId,
                    'X-Secure-Request': 'true'
                },
                body: encryptedBody
            };
            
            this.log("Sending secure request to: " + url, "debug");
            const response = await fetch(url, secureOptions);
            
            if (!response.ok) {
                throw new Error("Secure request failed: " + response.status);
            }
            
            // Decrypt response if it's encrypted
            const responseText = await response.text();
            let decryptedResponse;
            
            try {
                const responseData = JSON.parse(responseText);
                if (responseData.type === 'secure') {
                    decryptedResponse = await this.decrypt(responseData);
                    this.log("Response decrypted successfully", "debug");
                } else {
                    decryptedResponse = responseText;
                }
            } catch (e) {
                decryptedResponse = responseText; // Not JSON or not encrypted
            }
            
            // Return a Response-like object
            return {
                ok: response.ok,
                status: response.status,
                statusText: response.statusText,
                headers: response.headers,
                text: async () => decryptedResponse,
                json: async () => JSON.parse(decryptedResponse)
            };
            
        } catch (error) {
            this.log("Secure request failed: " + error.message, "error");
            throw error;
        }
    }
    
    async encrypt(plaintext) {
        if (!this.sharedKey) {
            throw new Error("Encryption key not available");
        }
        
        const iv = crypto.getRandomValues(new Uint8Array(12));
        const encoded = new TextEncoder().encode(plaintext);
        
        const encrypted = await crypto.subtle.encrypt(
            { name: "AES-GCM", iv: iv },
            this.sharedKey,
            encoded
        );
        
        const ciphertext = new Uint8Array(encrypted);
        const ciphertextHex = Array.from(ciphertext).map(b => 
            b.toString(16).padStart(2, '0')).join('');
        const ivHex = Array.from(iv).map(b => 
            b.toString(16).padStart(2, '0')).join('');
        
        return {
            type: 'secure',
            counter: ++this.messageCounter,
            data: ciphertextHex,
            iv: ivHex,
            tag: '' // Web Crypto includes tag in ciphertext
        };
    }
    
    async decrypt(encryptedData) {
        if (!this.sharedKey) {
            throw new Error("Decryption key not available");
        }
        
        const ciphertext = new Uint8Array(
            encryptedData.data.match(/.{2}/g).map(byte => parseInt(byte, 16))
        );
        const iv = new Uint8Array(
            encryptedData.iv.match(/.{2}/g).map(byte => parseInt(byte, 16))
        );
        
        const decrypted = await crypto.subtle.decrypt(
            { name: "AES-GCM", iv: iv },
            this.sharedKey,
            ciphertext
        );
        
        return new TextDecoder().decode(decrypted);
    }
    
    generateSessionId() {
        const array = new Uint8Array(16);
        crypto.getRandomValues(array);
        return Array.from(array).map(b => b.toString(16).padStart(2, '0')).join('');
    }
    
    isSecure() {
        return this.isEncrypted;
    }
    
    getSessionId() {
        return this.sessionId;
    }
    
    enableDebug(enabled = true) {
        this.debugMode = enabled;
    }
    
    log(message, level = 'info') {
        if (!this.debugMode && level === 'debug') return;
        
        const timestamp = new Date().toLocaleTimeString();
        const prefix = `[${timestamp}] [SecureClient] [${level.toUpperCase()}]`;
        
        switch (level) {
            case 'error':
                console.error(prefix, message);
                break;
            case 'warning':
                console.warn(prefix, message);
                break;
            case 'success':
                console.log('%c' + prefix + ' ' + message, 'color: green');
                break;
            case 'debug':
                console.log('%c' + prefix + ' ' + message, 'color: gray');
                break;
            default:
                console.log(prefix, message);
        }
    }
    
    // Compatibility wrapper for existing code
    async compatibleFetch(url, options = {}) {
        // Check if this endpoint should be secured
        const secureEndpoints = [
            '/api/keys',
            '/api/add',
            '/api/remove',
            '/api/config',     // 🔐 Server configuration
            '/api/keys/reorder', // 🔐 Keys reordering
            '/api/passwords',
            '/api/passwords/add',
            '/api/passwords/delete',
            '/api/passwords/get',
            '/api/passwords/update',
            '/api/passwords/reorder',
            '/api/passwords/export',
            '/api/passwords/import'
        ];
        
        const shouldSecure = secureEndpoints.some(endpoint => url.includes(endpoint));
        
        if (shouldSecure && this.isEncrypted) {
            return await this.makeSecureRequest(url, options);
        } else {
            return await fetch(url, options);
        }
    }
}

// Integration helpers
let globalSecureClient = null;

function initSecureClient(debugMode = false) {
    if (!globalSecureClient) {
        globalSecureClient = new SecureClient();
        globalSecureClient.enableDebug(debugMode);
    }
    return globalSecureClient;
}

function getSecureClient() {
    return globalSecureClient;
}

// Override global fetch for automatic security (optional)
function enableAutoSecure() {
    if (globalSecureClient) {
        const originalFetch = window.fetch;
        window.fetch = function(url, options) {
            return globalSecureClient.compatibleFetch(url, options);
        };
    }
}

// Modern API for secure requests
async function secureRequest(url, options = {}) {
    const client = getSecureClient();
    if (client) {
        return await client.makeSecureRequest(url, options);
    } else {
        throw new Error("SecureClient not initialized");
    }
}

// Export for module systems
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { SecureClient, initSecureClient, getSecureClient, secureRequest };
}
