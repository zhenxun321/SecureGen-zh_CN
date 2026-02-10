#include "web_server_secure_integration.h"
#include "log_manager.h"
#include "url_obfuscation_integration.h"
#include <ArduinoJson.h>

void WebServerSecureIntegration::addSecureEndpoints(AsyncWebServer& server, SecureLayerManager& secureLayer, URLObfuscationManager& urlObfuscation) {
    LOG_INFO("SecureIntegration", "Adding secure endpoints for HTTPS-like encryption");
    
    // Secure handshake endpoint
    server.on("/api/secure/hello", HTTP_GET, [&secureLayer](AsyncWebServerRequest *request) {
        LOG_DEBUG("🔐", "Hello endpoint hit");
        
        JsonDocument response;
        response["status"] = "ready";
        response["protocol_version"] = "1.0";
        response["encryption"] = "AES-256-GCM";
        response["key_exchange"] = "ECDH-P256";
        response["server_info"] = "SecureGen";
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(200, "application/json", responseStr);
    });
    
    // ECDH key exchange endpoint - register both original and obfuscated
    auto keyexchangeHandlerFunc = [&secureLayer](AsyncWebServerRequest *request) {
        LOG_INFO("🔐", "KeyExchange endpoint POST hit - headers check");
        String clientId = "";
        if (request->hasHeader("X-Client-ID")) {
            clientId = request->getHeader("X-Client-ID")->value();
            LOG_INFO("🔐", "X-Client-ID header found: " + clientId.substring(0,8) + "...");
        } else if (request->hasHeader("X-Req-UUID")) {
            clientId = request->getHeader("X-Req-UUID")->value();
            LOG_INFO("🔐", "X-Req-UUID header found (obfuscated): " + clientId.substring(0,8) + "...");
        } else {
            LOG_WARNING("🔐", "KeyExchange: Missing client ID header");
        }
    };
    
    auto keyexchangeBodyFunc = [&secureLayer](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        if (index + len == total) {
            LOG_INFO("🔐", "KeyExchange body received: " + String(len) + " bytes");
            
            // Parse client data
            String body = String((char*)data, len);
            LOG_DEBUG("🔐", "KeyExchange request body: " + body.substring(0, 100) + "...");
            
            JsonDocument doc;
            
            if (deserializeJson(doc, body) != DeserializationError::Ok) {
                LOG_ERROR("🔐", "KeyExchange: Invalid JSON in body");
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            
            LOG_INFO("🔐", "KeyExchange JSON parsed successfully");
            
            if (!doc["client_id"].is<String>() || !doc["client_public_key"].is<String>()) {
                LOG_ERROR("🔐", "KeyExchange: Missing required fields");
                if (!doc["client_id"].is<String>()) LOG_ERROR("🔐", "Missing client_id field");
                if (!doc["client_public_key"].is<String>()) LOG_ERROR("🔐", "Missing client_public_key field");
                request->send(400, "application/json", "{\"error\":\"Missing required fields\"}");
                return;
            }
            
            String clientId = doc["client_id"].as<String>();
            String clientPubKey = doc["client_public_key"].as<String>();
            
            LOG_INFO("🔐", "KeyExchange processing for client: " + clientId.substring(0,8) + "...");
            LOG_DEBUG("🔐", "Client public key length: " + String(clientPubKey.length()));
            
            String response;
            if (secureLayer.processKeyExchange(clientId, clientPubKey, response)) {
                LOG_INFO("🔐", "KeyExchange SUCCESS - sending response");
                request->send(200, "application/json", response);
            } else {
                LOG_ERROR("🔐", "KeyExchange FAILED in processKeyExchange()");
                request->send(500, "application/json", "{\"error\":\"Key exchange failed\"}");
            }
        } else {
            LOG_DEBUG("🔐", "KeyExchange partial data: " + String(index) + "/" + String(total));
        }
    };
    
    // Register original endpoint
    server.on("/api/secure/keyexchange", HTTP_POST, keyexchangeHandlerFunc, NULL, keyexchangeBodyFunc);
    
    // Register obfuscated endpoint
    String obfuscatedPath = urlObfuscation.obfuscateURL("/api/secure/keyexchange");
    if (obfuscatedPath.length() > 0 && obfuscatedPath != "/api/secure/keyexchange") {
        server.on(obfuscatedPath.c_str(), HTTP_POST, keyexchangeHandlerFunc, NULL, keyexchangeBodyFunc);
    }
    
    // Secure session status endpoint
    server.on("/api/secure/status", HTTP_GET, [&secureLayer](AsyncWebServerRequest *request) {
        String clientId = "";
        if (request->hasHeader("X-Client-ID")) {
            clientId = request->getHeader("X-Client-ID")->value();
        } else if (request->hasHeader("X-Req-UUID")) {
            clientId = request->getHeader("X-Req-UUID")->value();
        }
        
        JsonDocument response;
        response["secure_sessions"] = secureLayer.getActiveSecureSessionCount();
        response["max_sessions"] = SECURE_MAX_SESSIONS;
        
        if (clientId.length() > 0) {
            response["client_session_active"] = secureLayer.isSecureSessionValid(clientId);
        }
        
        response["memory_usage"] = ESP.getFreeHeap();
        
        String responseStr;
        serializeJson(response, responseStr);
        
        request->send(200, "application/json", responseStr);
    });
    
    // Secure test endpoint - encrypts a test message
    server.on("/api/secure/test", HTTP_POST, [&secureLayer](AsyncWebServerRequest *request) {
        LOG_DEBUG("SecureIntegration", "Secure test endpoint called");
    }, NULL, [&secureLayer](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        
        if (index + len == total) {
            String clientId = "";
            if (request->hasHeader("X-Client-ID")) {
                clientId = request->getHeader("X-Client-ID")->value();
            } else if (request->hasHeader("X-Req-UUID")) {
                clientId = request->getHeader("X-Req-UUID")->value();
            }
            
            if (clientId.length() == 0) {
                request->send(400, "application/json", "{\"error\":\"Missing client ID header\"}");
                return;
            }
            
            if (!secureLayer.isSecureSessionValid(clientId)) {
                request->send(401, "application/json", "{\"error\":\"No secure session established\"}");
                return;
            }
            
            String body = String((char*)data, len);
            JsonDocument doc;
            
            if (deserializeJson(doc, body) != DeserializationError::Ok) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            
            String testMessage = "HTTPS Test: Server time " + String(millis()) + "ms";
            if (doc["message"].is<String>()) {
                testMessage = doc["message"].as<String>();
            }
            
            String encryptedResponse;
            if (secureLayer.encryptResponse(clientId, testMessage, encryptedResponse)) {
                LOG_INFO("SecureIntegration", "Test message encrypted successfully");
                request->send(200, "application/json", encryptedResponse);
            } else {
                LOG_ERROR("SecureIntegration", "Failed to encrypt test message");
                request->send(500, "application/json", "{\"error\":\"Encryption failed\"}");
            }
        }
    });
    
    LOG_INFO("SecureIntegration", "All secure endpoints added successfully");
}

bool WebServerSecureIntegration::processSecureRequest(AsyncWebServerRequest* request, SecureLayerManager& secureLayer, String& processedBody) {
    String clientId = "";
    if (request->hasHeader("X-Client-ID")) {
        clientId = request->getHeader("X-Client-ID")->value();
    } else if (request->hasHeader("X-Req-UUID")) {
        clientId = request->getHeader("X-Req-UUID")->value();
    }
    
    // Check for simple/fallback encryption mode
    bool isSimpleSecure = request->hasHeader("X-Simple-Secure");
    // ИСПРАВЛЕНИЕ: Обфусцированные заголовки + принудительное шифрование для валидных сессий
    bool isFullSecure = request->hasHeader("X-Secure-Request") || request->hasHeader("X-Security-Level") || 
                       (clientId.length() > 0 && secureLayer.isSecureSessionValid(clientId));
    
    if (isSimpleSecure) {
        // Simple fallback encryption mode - don't validate secure session
        LOG_DEBUG("🔐", "Fallback decrypt: " + clientId.substring(0,8) + "...");
        // For simple mode, we don't decrypt the body, just pass it through
        // The body is already base64 encoded by client
        return true;
    }
    
    if (clientId.length() == 0 || !secureLayer.isSecureSessionValid(clientId)) {
        return false; // No secure session, process as regular request
    }
    
    if (isFullSecure) {
        LOG_DEBUG("🔐", "Full AES decrypt: " + clientId.substring(0,8) + "...");
    }
    
    return true;
}

// ⚡ IRAM_ATTR - hot-path функция HTTP обработки
IRAM_ATTR void WebServerSecureIntegration::sendSecureResponse(AsyncWebServerRequest* request, int code, const String& contentType, const String& content, SecureLayerManager& secureLayer) {
    String clientId = "";
    if (request->hasHeader("X-Client-ID")) {
        clientId = request->getHeader("X-Client-ID")->value();
    } else if (request->hasHeader("X-Req-UUID")) {
        clientId = request->getHeader("X-Req-UUID")->value();
    }
    
    // Check for simple/fallback encryption mode
    bool isSimpleSecure = request->hasHeader("X-Simple-Secure");
    // ИСПРАВЛЕНИЕ: Обфусцированные заголовки + принудительное шифрование для валидных сессий
    bool isFullSecure = request->hasHeader("X-Secure-Request") || request->hasHeader("X-Security-Level") || 
                       (clientId.length() > 0 && secureLayer.isSecureSessionValid(clientId));
    
    if (isSimpleSecure) {
        // Simple fallback encryption - just encode response with Base64
        LOG_DEBUG("🔐", "Fallback encrypt: " + String(content.length()) + "b → " + clientId.substring(0,8) + "...");
        
        // Simple Base64 encoding for fallback encryption
        String base64Content = "";
        size_t len = content.length();
        const char* chars = content.c_str();
        
        // Simple Base64 encoding (basic implementation)
        const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (size_t i = 0; i < len; i += 3) {
            unsigned char a = chars[i];
            unsigned char b = (i + 1 < len) ? chars[i + 1] : 0;
            unsigned char c = (i + 2 < len) ? chars[i + 2] : 0;
            
            unsigned int bitmap = (a << 16) | (b << 8) | c;
            
            base64Content += base64_chars[(bitmap >> 18) & 0x3F];
            base64Content += base64_chars[(bitmap >> 12) & 0x3F];
            base64Content += (i + 1 < len) ? base64_chars[(bitmap >> 6) & 0x3F] : '=';
            base64Content += (i + 2 < len) ? base64_chars[bitmap & 0x3F] : '=';
        }
        
        request->send(code, contentType, base64Content);
        return;
    }
    
    if (clientId.length() == 0 || !secureLayer.isSecureSessionValid(clientId)) {
        // No secure session, send regular response
        request->send(code, contentType, content);
        return;
    }
    
    if (isFullSecure) {
        // Full AES-GCM encryption
        String encryptedContent;
        if (secureLayer.encryptResponse(clientId, content, encryptedContent)) {
            request->send(code, "application/json", encryptedContent);
        } else {
            LOG_ERROR("🔐", "Full encrypt FAILED");
            request->send(500, "application/json", "{\"error\":\"Encryption failed\"}");
        }
        return;
    }
    
    // Default: send regular response
    request->send(code, contentType, content);
}

String WebServerSecureIntegration::getClientId(AsyncWebServerRequest* request) {
    // Check original header first
    if (request->hasHeader("X-Client-ID")) {
        return request->getHeader("X-Client-ID")->value();
    }
    
    // Check obfuscated header mapping (Header Obfuscation support)
    if (request->hasHeader("X-Req-UUID")) {
        return request->getHeader("X-Req-UUID")->value();
    }
    
    return "";
}
