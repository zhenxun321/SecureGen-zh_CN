# Reddit Post Template for SecureGen

## 📋 Recommended Subreddits (in order of priority):

1. **r/esp32** - Main ESP32 community (most relevant)
2. **r/arduino** - Arduino/embedded projects
3. **r/embedded** - Embedded systems
4. **r/cybersecurity** - Security focus
5. **r/privacy** - Privacy-focused audience
6. **r/selfhosted** - Self-hosted solutions
7. **r/opensource** - Open source projects
8. **r/LILYGO** - LILYGO hardware specific
9. **r/maker** - Maker community
10. **r/electronics** - Electronics enthusiasts

---

## 📝 Post Title Options:

**Option 1 (Technical):**
```
[Project] SecureGen - Open-source Hardware TOTP Authenticator & Password Manager on ESP32 T-Display
```

**Option 2 (Feature-focused):**
```
I built a hardware 2FA authenticator with password manager using ESP32 T-Display (Open Source)
```

**Option 3 (Problem-solving):**
```
Tired of phone-based authenticators? Built an offline hardware TOTP device with ESP32
```

---

## 📄 Post Content (Markdown format for Reddit):

```markdown
# SecureGen - Hardware TOTP Authenticator & Password Manager

Hi r/esp32! I've been working on an open-source security device and just released v1.0.0. Thought you might find it interesting!

## 🔐 What is it?

SecureGen is a hardware-based TOTP authenticator (like Google Authenticator) combined with a password manager, built on the LILYGO ESP32 T-Display. It's completely offline and air-gapped for maximum security.

## ✨ Key Features

- **TOTP Authenticator** - RFC 6238 compliant, 30/60 second codes
- **Password Manager** - Encrypted vault with 256-bit AES
- **BLE Keyboard** - Types passwords directly into your computer
- **7+ Security Layers** - PBKDF2, traffic obfuscation, anti-timing attacks
- **Web Interface** - Easy management via WiFi
- **Battery Powered** - Portable with JST battery connector
- **Dual Themes** - Light/Dark mode support
- **Offline First** - No cloud, no internet required for operation

## 🎯 Why I Built This

I wanted a physical 2FA device that:
- Doesn't rely on my phone
- Works completely offline
- Stores passwords securely
- Is open source and auditable
- Can type passwords via BLE keyboard

## 📦 What's Included

The v1.0.0 release includes:
- Pre-compiled firmware binaries (ready to flash)
- Full source code
- Documentation and setup guides
- Web interface for management

## 🛠️ Hardware Requirements

- LILYGO T-Display ESP32 (~$15 on AliExpress)
- USB-C cable for programming
- Optional: 3.7V LiPo battery with JST connector

## 🚀 Installation

Three options:
1. **Flash pre-built binaries** (easiest - just download and flash)
2. **Build from source** with PlatformIO
3. **Use Arduino IDE**

Full instructions in the repo!

## 📸 Screenshots

[Add your screenshots here when you have them]

## 🔗 Links

- **GitHub Repository:** https://github.com/makepkg/SecureGen
- **Latest Release:** https://github.com/makepkg/SecureGen/releases/tag/v1.0.0
- **Documentation:** https://github.com/makepkg/SecureGen/blob/master/README.md

## 🤝 Contributing

This is fully open source (MIT License). Contributions, bug reports, and feature requests are welcome!

## 💭 Questions?

Happy to answer any questions about the project, security architecture, or implementation details!

---

**Tech Stack:** ESP32, Arduino Framework, TFT_eSPI, BLE, AES-256, PBKDF2, ECDH

**License:** MIT

If you find this useful, a ⭐ on GitHub would be appreciated!
```

---

## 🎨 Post with Images (Alternative format):

If you have screenshots ready, use this format:

```markdown
# [Project] SecureGen - Hardware TOTP + Password Manager on ESP32

[Image 1: Device showing TOTP code]

Just released v1.0.0 of my open-source hardware authenticator project!

**What it does:**
- Generates TOTP codes (like Google Authenticator)
- Stores passwords in encrypted vault
- Types passwords via BLE keyboard
- Completely offline and air-gapped

[Image 2: Web interface]

**Hardware:** LILYGO T-Display ESP32 (~$15)

**Security:** 7+ layers including AES-256, PBKDF2, traffic obfuscation

[Image 3: Different screens/modes]

**Features:**
✅ Battery powered & portable
✅ Web-based management
✅ Light/Dark themes
✅ Factory reset protection
✅ PIN code lock

**Download:** Pre-built firmware ready to flash!

🔗 GitHub: https://github.com/makepkg/SecureGen
📦 Release: https://github.com/makepkg/SecureGen/releases/tag/v1.0.0

Open source (MIT) - contributions welcome!

AMA about the project! 🚀
```

---

## 📊 Posting Strategy:

### Timing:
- **Best days:** Tuesday-Thursday
- **Best time:** 8-10 AM EST or 2-4 PM EST
- Avoid weekends for technical subs

### Engagement Tips:
1. **Respond quickly** to comments in first 2 hours
2. **Be helpful** - answer technical questions
3. **Share details** about challenges you faced
4. **Add flair** if subreddit requires it
5. **Cross-post** after 24 hours to other relevant subs

### What to Prepare:
- [ ] High-quality photos of device
- [ ] GIF/video of it in action (optional but great)
- [ ] Screenshots of web interface
- [ ] Be ready to answer questions about:
  - Security implementation
  - Why not use phone apps
  - Hardware cost
  - Battery life
  - Comparison to YubiKey/other devices

### Follow Reddit Rules:
- Check each subreddit's rules before posting
- Some require [Project] or [OC] tags
- Some have specific days for project posts
- Don't spam multiple subs at once (space out by 24h)

---

## 🎯 Expected Questions & Answers:

**Q: Why not just use Google Authenticator on phone?**
A: This is for people who want a dedicated, offline device. Phone can be lost, hacked, or out of battery. This is air-gapped and purpose-built.

**Q: How is this different from YubiKey?**
A: YubiKey is hardware-only (no screen). This has a display showing codes, stores passwords, and can type them via BLE. Also it's open source and $15 vs $50+.

**Q: Is it secure to have WiFi on a security device?**
A: WiFi is only for initial setup and management. During normal operation, you can disable WiFi completely. TOTP generation works 100% offline.

**Q: Battery life?**
A: Several days of active use, weeks on standby. Depends on usage patterns and battery capacity.

**Q: Can I audit the security?**
A: Yes! Full source code is available. Security documentation explains all layers. That's the point of open source.

---

## 📈 Success Metrics:

Good post performance:
- 50+ upvotes in first 24h
- 10+ comments with questions
- 5+ GitHub stars from Reddit traffic

Great post performance:
- 200+ upvotes
- 30+ comments
- 20+ GitHub stars
- Cross-posts to other subs

---

## 🔄 Follow-up Posts (Future):

After initial release, you can post updates:
- "v1.1.0 released with [new features]"
- "Tutorial: How I implemented [specific feature]"
- "Lessons learned building a hardware security device"
- "Community requested features now live"

---

**Ready to post? Start with r/esp32 - it's the most relevant community!**

Good luck! 🚀
