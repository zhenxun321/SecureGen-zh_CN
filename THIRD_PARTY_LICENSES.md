# Third-Party Licenses

This project uses the following open-source libraries and components. We are grateful to their authors and contributors.

---

## Direct Dependencies

### TFT_eSPI
- **Version:** 2.5.43
- **License:** FreeBSD License
- **Author:** Bodmer
- **Repository:** https://github.com/Bodmer/TFT_eSPI
- **Purpose:** Display driver for ST7789 TFT screen

**License Text:**
```
FreeBSD License

Copyright (c) Bodmer

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
```

---

### ESPAsyncWebServer
- **Version:** 1.2.4
- **License:** LGPL-3.0
- **Author:** me-no-dev
- **Repository:** https://github.com/me-no-dev/ESPAsyncWebServer
- **Purpose:** Asynchronous web server for ESP32

**License:** GNU Lesser General Public License v3.0
- Full text: https://www.gnu.org/licenses/lgpl-3.0.html

---

### AsyncTCP
- **Version:** 1.1.1
- **License:** LGPL-3.0
- **Author:** me-no-dev
- **Repository:** https://github.com/me-no-dev/AsyncTCP
- **Purpose:** Asynchronous TCP library for ESP32

**License:** GNU Lesser General Public License v3.0
- Full text: https://www.gnu.org/licenses/lgpl-3.0.html

---

### ArduinoJson
- **Version:** 7.4.2
- **License:** MIT License
- **Author:** Benoit Blanchon
- **Repository:** https://github.com/bblanchon/ArduinoJson
- **Purpose:** JSON parsing and serialization

**License Text:**
```
MIT License

Copyright (c) 2014-2024 Benoit BLANCHON

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Platform Dependencies (Included in ESP-IDF)

### mbedTLS
- **License:** Apache License 2.0
- **Author:** ARM Limited
- **Repository:** https://github.com/Mbed-TLS/mbedtls
- **Purpose:** Cryptographic library (AES, SHA, ECDH, etc.)

**License:** Apache License 2.0
- Full text: https://www.apache.org/licenses/LICENSE-2.0

---

### ESP-IDF (Espressif IoT Development Framework)
- **License:** Apache License 2.0
- **Author:** Espressif Systems
- **Repository:** https://github.com/espressif/esp-idf
- **Purpose:** ESP32 development framework

**License:** Apache License 2.0
- Full text: https://www.apache.org/licenses/LICENSE-2.0

---

### FreeRTOS
- **License:** MIT License
- **Author:** Amazon Web Services
- **Repository:** https://github.com/FreeRTOS/FreeRTOS-Kernel
- **Purpose:** Real-time operating system kernel

**License:** MIT License (included in ESP-IDF)

---

## Hardware

### LILYGO T-Display
- **Manufacturer:** LILYGO
- **Product:** TTGO T-Display ESP32
- **Website:** https://www.lilygo.cc/

Hardware design and schematics are provided by LILYGO under their terms.

---

## Compliance Notes

### LGPL-3.0 Compliance (ESPAsyncWebServer, AsyncTCP)

This project uses ESPAsyncWebServer and AsyncTCP which are licensed under LGPL-3.0. 

**LGPL-3.0 allows:**
- Use in proprietary software
- Dynamic linking without source disclosure
- Modifications to LGPL libraries must be disclosed

**Our compliance:**
- We use these libraries as-is without modifications
- Libraries are dynamically linked via PlatformIO
- Source code of these libraries is available in their repositories
- Users can replace these libraries with compatible alternatives

### Apache 2.0 Compliance (mbedTLS, ESP-IDF)

**Apache 2.0 allows:**
- Commercial use
- Modification
- Distribution
- Patent use

**Our compliance:**
- We include required notices
- We acknowledge the use of Apache-licensed components
- We do not claim endorsement by Apache or component authors

---

## Attribution

We thank all the authors and contributors of the above projects for their excellent work that makes this project possible.

---

## Questions?

If you have questions about licensing or compliance, please:
- Open a [Discussion](https://github.com/Unix-like-SoN/SecureGen/discussions)
- Contact the maintainer via GitHub

---

**Last Updated:** February 2025
