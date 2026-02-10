# 🛡️ Security Overview - ESP32 T-Display TOTP

**Last Updated:** February 2025  
**Security Level:** High  
**Target Audience:** Public

---

## 📋 Table of Contents

1. [Security Philosophy](#security-philosophy)
2. [Encryption & Data Protection](#encryption--data-protection)
3. [Authentication & Authorization](#authentication--authorization)
4. [Web Server Security](#web-server-security)
5. [Bluetooth Security](#bluetooth-security)
6. [Physical Security](#physical-security)
7. [Security Best Practices](#security-best-practices)
8. [Responsible Disclosure](#responsible-disclosure)

---

## Security Philosophy

This device is designed with **security-first** principles for protecting sensitive authentication data and passwords. Our approach follows industry standards while adapting to the constraints of embedded hardware.

### Core Principles

- **Defense in Depth:** Multiple layers of security
- **Encryption at Rest:** All sensitive data encrypted on storage
- **Encryption in Transit:** Secure communication channels
- **Minimal Attack Surface:** Only essential features enabled
- **Physical Security:** PIN protection and secure boot options
- **Open Source:** Transparent security implementation for community review

---

## Encryption & Data Protection

### Data Encryption at Rest

All sensitive data stored on the device is encrypted using **military-grade encryption**:

- **Algorithm:** AES-256 in CBC mode
- **Key Management:** Unique device key generated from hardware parameters
- **Key Storage:** Securely stored in encrypted filesystem
- **Padding:** Industry-standard PKCS#7

**Protected Data:**
- TOTP secrets
- Stored passwords
- WiFi credentials
- Administrator credentials
- Session tokens
- BLE pairing information

### Password-Based Encryption

For import/export operations, an additional layer of password-based encryption protects your backup files:

- **Algorithm:** AES-256-CBC
- **Key Derivation:** PBKDF2-HMAC-SHA256
- **Salt:** Unique random salt for each backup
- **Iterations:** Optimized for ESP32 performance while maintaining security

This ensures that even if backup files are intercepted, they cannot be decrypted without your password.

### Hardware-Accelerated Cryptography

The ESP32 chip includes hardware acceleration for cryptographic operations:

- ✅ AES encryption/decryption
- ✅ SHA-256 hashing
- ✅ Elliptic Curve operations
- ✅ Hardware random number generator

This provides both **better performance** and **enhanced security** compared to software-only implementations.

---

## Authentication & Authorization

### Password Security

User passwords are never stored in plain text. Instead, we use:

- **Algorithm:** PBKDF2-HMAC-SHA256
- **Salt:** Unique random salt per password
- **Iterations:** Tuned for optimal security on ESP32 hardware
- **Output:** 256-bit cryptographic hash

This makes brute-force attacks computationally infeasible.

### Session Management

Web interface sessions are secured with:

- **Session IDs:** 256-bit cryptographically random tokens
- **CSRF Protection:** Unique tokens for each session
- **Timeout:** Configurable session expiration (default: 6 hours)
- **Persistent Sessions:** Optional "remember me" functionality
- **Automatic Cleanup:** Expired sessions are automatically removed

### Brute-Force Protection

The system includes protection against password guessing attacks:

- **Rate Limiting:** Maximum login attempts before lockout
- **Temporary Lockout:** Time-based blocking after failed attempts
- **Automatic Reset:** Counters reset after successful authentication

---

## Web Server Security

### Multi-Layer Encryption System

The web interface implements a sophisticated **HTTPS-equivalent** security system over HTTP:

#### Layer 1: Key Exchange
- **Protocol:** Elliptic Curve Diffie-Hellman (ECDH)
- **Curve:** NIST P-256 (industry standard)
- **Purpose:** Establish secure session keys

#### Layer 2: Message Encryption
- **Algorithm:** AES-256-GCM (Authenticated Encryption)
- **Features:**
  - Confidentiality (encryption)
  - Integrity (authentication tags)
  - Replay protection (message counters)

#### Layer 3: URL Obfuscation
- **Method:** Dynamic path generation
- **Purpose:** Hide API endpoints from network observers
- **Rotation:** Paths change on device reboot

#### Layer 4: Header Obfuscation
- **Method:** Dynamic header name mapping
- **Purpose:** Obscure HTTP metadata
- **Includes:** Fake headers to confuse traffic analysis

#### Layer 5: Method Tunneling
- **Method:** HTTP method override
- **Purpose:** Additional protocol-level obfuscation
- **Application:** Critical operations (delete, update)

### Why Not Standard HTTPS?

Standard TLS/HTTPS requires significant memory resources (40-60KB RAM for handshake) which would severely limit functionality on the ESP32's 520KB RAM. Our multi-layer approach provides **equivalent security** while maintaining full feature set.

### Network Isolation

The device supports multiple network modes for different security requirements:

- **Offline Mode:** Complete air-gap operation (maximum security)
- **AP Mode:** Isolated WiFi network for configuration
- **Client Mode:** Connect to trusted network only

---

## Bluetooth Security

### BLE Pairing & Encryption

Bluetooth Low Energy communication uses **LE Secure Connections** with:

- **Authentication:** PIN-based pairing with MITM protection
- **Encryption:** AES-128 (BLE standard)
- **Bonding:** Trusted devices remembered for future connections
- **Key Storage:** Pairing keys securely stored in NVS

### PIN Protection

- **Display:** 6-digit PIN shown on device screen
- **Entry:** User enters PIN on connecting device
- **Rotation:** PIN can be changed, invalidating all bonded devices
- **Generation:** Cryptographically random PIN generation

### Transmission Security

When sending passwords via BLE:

- ✅ Encrypted connection required
- ✅ Authenticated pairing verified
- ✅ Automatic timeout after transmission
- ✅ Optional additional PIN protection

---

## Physical Security

### Device PIN Protection

Protect your device from unauthorized physical access:

- **Startup PIN:** Lock device on boot (4-10 digits)
- **BLE PIN:** Protect wireless password transmission
- **Configurable:** Enable/disable independently
- **Secure Storage:** PIN hashes encrypted on device

### Factory Reset Protection

Secure data wiping when needed:

- **Activation:** Hold both buttons for 5 seconds on boot
- **Complete Wipe:** All data securely erased
- **Includes:**
  - All TOTP secrets and passwords
  - WiFi credentials
  - Session data
  - BLE bonding keys
  - Device configuration

### Power Management Security

Battery-powered operation includes security considerations:

- **Auto-Sleep:** Device sleeps after inactivity
- **Wake Protection:** Requires button press to wake
- **Screen Timeout:** Automatic screen lock
- **BLE Timeout:** Automatic disconnection after transmission

---

## Security Best Practices

### For Users

**Network Security:**
- Use strong WiFi passwords (WPA3 if available)
- Consider network isolation for the device
- Avoid public WiFi for device configuration

**Physical Security:**
- Enable PIN protection for startup
- Keep device physically secure
- Use factory reset if device is compromised

**Data Management:**
- Regularly export encrypted backups
- Store backups in secure location
- Use strong passwords for backup encryption
- Change PIN codes periodically

**Session Management:**
- Log out from web interface when finished
- Use appropriate session timeout settings
- Clear bonded BLE devices periodically

### For Developers

**Code Security:**
- Review security-critical code changes carefully
- Test cryptographic implementations thoroughly
- Follow secure coding practices
- Keep dependencies updated

**Deployment:**
- Use latest firmware version
- Enable all security features
- Configure appropriate timeouts
- Monitor for security updates

---

## Known Limitations

We believe in transparency about security limitations:

### Hardware Constraints

- **PBKDF2 Iterations:** Lower than server-grade recommendations due to ESP32 performance limits
  - Mitigated by: Hardware acceleration, rate limiting, physical security
  
- **Memory Limitations:** 520KB RAM limits some security features
  - Mitigated by: Efficient implementation, multi-layer approach

- **BLE Encryption:** AES-128 (BLE standard limitation)
  - Mitigated by: LE Secure Connections, MITM protection, bonding

### Design Decisions

- **HTTP vs HTTPS:** Multi-layer encryption over HTTP instead of TLS
  - Reason: Memory constraints
  - Mitigation: Equivalent security through layered approach

- **Custom Splash Disabled:** Upload feature removed
  - Reason: Potential security vulnerability
  - Alternative: Built-in splash screens only

---

## Security Updates

### Update Policy

- Security patches released as soon as possible
- Critical vulnerabilities addressed immediately
- Regular security audits performed
- Community feedback actively monitored

### Checking for Updates

- Monitor GitHub repository for releases
- Review changelog for security fixes
- Subscribe to security notifications
- Join community discussions

---

## Responsible Disclosure

### Reporting Security Issues

If you discover a security vulnerability, please report it responsibly:

**DO:**
- Email security concerns privately to the maintainer
- Provide detailed information about the vulnerability
- Allow reasonable time for fix before public disclosure
- Work with us to verify the fix

**DON'T:**
- Publicly disclose vulnerabilities before they're fixed
- Exploit vulnerabilities maliciously
- Test vulnerabilities on devices you don't own

### Contact

- **GitHub Issues:** For non-critical security discussions
- **Private Contact:** For critical vulnerabilities (see repository)

### Recognition

Security researchers who responsibly disclose vulnerabilities will be:
- Credited in release notes (if desired)
- Listed in security acknowledgments
- Thanked for improving the project

---

## Compliance & Standards

### Cryptographic Standards

Our implementation uses industry-standard algorithms:

- ✅ AES-256 (FIPS 197)
- ✅ SHA-256 (FIPS 180-4)
- ✅ PBKDF2 (RFC 2898, NIST SP 800-132)
- ✅ ECDH P-256 (FIPS 186-4)
- ✅ AES-GCM (NIST SP 800-38D)

### Best Practices Alignment

- ✅ OWASP Top 10 considerations
- ✅ NIST Digital Identity Guidelines (partial)
- ✅ Defense in depth principles
- ✅ Least privilege access
- ✅ Secure by default configuration

---

## Security Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Physical Device                          │
│  ┌────────────────────────────────────────────────────────┐ │
│  │  PIN Protection → Device Key → AES-256 Encryption      │ │
│  │                                                         │ │
│  │  ┌──────────────┐  ┌──────────────┐  ┌─────────────┐ │ │
│  │  │ TOTP Secrets │  │  Passwords   │  │ WiFi Creds  │ │ │
│  │  │  (Encrypted) │  │  (Encrypted) │  │ (Encrypted) │ │ │
│  │  └──────────────┘  └──────────────┘  └─────────────┘ │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │      Secure Communication Layers         │
        │                                          │
        │  Layer 5: Method Tunneling              │
        │  Layer 4: Header Obfuscation            │
        │  Layer 3: URL Obfuscation               │
        │  Layer 2: AES-256-GCM Encryption        │
        │  Layer 1: ECDH Key Exchange             │
        └─────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │         Web Interface / BLE              │
        │                                          │
        │  • Session Management (CSRF)            │
        │  • Authentication (PBKDF2)              │
        │  • Rate Limiting                        │
        │  • BLE Encryption (AES-128)             │
        └─────────────────────────────────────────┘
```

---

## Conclusion

This device implements **enterprise-grade security** adapted for embedded hardware constraints. While no system is perfectly secure, we've implemented multiple layers of protection following industry best practices.

**Security is a continuous process.** We actively monitor for vulnerabilities, welcome community feedback, and regularly update our security measures.

**Your security is our priority.** Use this device with confidence, follow best practices, and report any concerns.

---

## Additional Resources

- [Full Documentation](../README.md)
- [Security Best Practices Guide](../README.md#security-best-practices)
- [GitHub Repository](https://github.com/Unix-like-SoN/SecureGen)
- [Community Discussions](https://github.com/Unix-like-SoN/SecureGen/discussions)

---

**Last Security Audit:** February 2025  
**Next Scheduled Audit:** August 2025

---

*This document provides a high-level overview of security features. Detailed implementation specifics are available in the source code for security researchers and auditors.*
