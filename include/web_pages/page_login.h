#pragma once

const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>登录 - T-Display TOTP</title>
    <link rel="icon" type="image/svg+xml" href="/favicon.svg">
    <link rel="alternate icon" href="/favicon.ico">
    <style>
        @keyframes gradient-animation {
            0% { background-position: 0% 50%; }
            50% { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
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
            height: 100vh;
            margin: 0;
        }

        .container {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            padding: 2.5rem;
            border-radius: 15px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
            width: 350px;
            text-align: center;
        }

        h2 {
            margin-bottom: 2rem;
            color: #ffffff;
            font-weight: 300;
            letter-spacing: 1px;
        }

        .input-group {
            margin-bottom: 1.5rem;
            text-align: left;
            position: relative;
        }

        label {
            display: block;
            margin-bottom: 0.5rem;
            color: #b0b0b0;
            font-size: 0.9rem;
        }

        input {
            width: 100%;
            padding: 0.8rem;
            background-color: rgba(0, 0, 0, 0.2);
            border: 1px solid rgba(255, 255, 255, 0.2);
            color: #e0e0e0;
            border-radius: 8px;
            box-sizing: border-box;
            transition: all 0.3s ease;
        }

        input:focus {
            outline: none;
            border-color: #5a9eee;
            box-shadow: 0 0 0 3px rgba(90, 158, 238, 0.3);
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
        }

        button:hover {
            background-color: #4a8bdb;
        }
        
        button:disabled {
            background-color: #555;
            color: #888;
            cursor: not-allowed;
        }

        .error-message {
            color: #e57373;
            background: rgba(244, 67, 54, 0.2);
            border: 1px solid rgba(244, 67, 54, 0.3);
            padding: 10px;
            border-radius: 8px;
            margin-top: 1rem;
            display: none;
            backdrop-filter: blur(10px);
        }

        .lockout-message {
            color: #ffb74d;
            background: rgba(255, 193, 7, 0.2);
            border: 1px solid rgba(255, 193, 7, 0.3);
            padding: 10px;
            border-radius: 8px;
            margin-top: 1rem;
            display: none;
            backdrop-filter: blur(10px);
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
    </style>
</head>
<body>
    <div class="container">
        <h2>管理员登录</h2>
        <form action="/login" method="post">
            <div class="input-group">
                <label for="username">用户名</label>
                <input type="text" id="username" name="username" required>
            </div>
            <div class="input-group">
                <label for="password">密码</label>
                <div class="password-input-container">
                    <input type="password" id="password" name="password" required>
                    <span class="password-toggle" onclick="togglePasswordVisibility('password', this)">👁</span>
                </div>
            </div>
            <button type="submit" id="loginButton">登录</button>
        </form>
        <p id="errorMessage" class="error-message">用户名或密码错误。</p>
        <p id="lockoutMessage" class="lockout-message"></p>
    </div>

    <script>
        // 🔐 XOR шифрование метода для защиты Login
        function generateSessionId() {
            return Array.from(crypto.getRandomValues(new Uint8Array(16)))
                .map(b => b.toString(16).padStart(2, '0')).join('');
        }

        function xorEncrypt(data, key) {
            let result = '';
            for (let i = 0; i < data.length; i++) {
                const charCode = data.charCodeAt(i) ^ key.charCodeAt(i % key.length);
                result += charCode.toString(16).padStart(2, '0');
            }
            return result;
        }

        // 🔐 XOR шифрование тела запроса (JSON)
        function xorEncryptBody(jsonData, key) {
            const jsonString = JSON.stringify(jsonData);
            return xorEncrypt(jsonString, key);
        }

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

        document.addEventListener('DOMContentLoaded', async function() {
            // 🔗 ЗАГРУЖАЕМ URL OBFUSCATION MAPPINGS
            try {
                console.log('🔗 Loading URL obfuscation mappings for login...');
                const response = await fetch('/api/url_obfuscation/mappings');
                if (response.ok) {
                    const mappings = await response.json();
                    window.urlObfuscationMap = mappings;
                    console.log(`🔗 Loaded ${Object.keys(mappings).length} URL mappings`);
                } else {
                    console.warn('⚠️ Failed to load URL mappings, using direct URLs');
                    window.urlObfuscationMap = {};
                }
            } catch (error) {
                console.warn('⚠️ Error loading URL mappings:', error.message);
                window.urlObfuscationMap = {};
            }
            
            const urlParams = new URLSearchParams(window.location.search);
            const error = urlParams.get('error');
            const lockoutTime = urlParams.get('time');
            const errorMessage = document.getElementById('errorMessage');
            const lockoutMessage = document.getElementById('lockoutMessage');
            const loginButton = document.getElementById('loginButton');
            const form = document.querySelector('form');
            
            // 🔗 Обновляем form action если есть obfuscated URL
            if (window.urlObfuscationMap && window.urlObfuscationMap['/login']) {
                form.action = window.urlObfuscationMap['/login'];
                console.log(`🔗 Form action updated: /login -> ${form.action}`);
            }

            if (error === '1') {
                errorMessage.style.display = 'block';
            }

            if (error === '2' && lockoutTime) {
                let timeLeft = parseInt(lockoutTime, 10);
                lockoutMessage.style.display = 'block';
                loginButton.disabled = true;
                
                const updateTimer = () => {
                    if (timeLeft > 0) {
                        lockoutMessage.textContent = `Too many failed attempts. Please try again in ${timeLeft} second(s).`;
                        timeLeft--;
                    } else {
                        clearInterval(timerInterval);
                        lockoutMessage.style.display = 'none';
                        loginButton.disabled = false;
                        // Очищаем URL от параметров ошибки, чтобы сообщение не появилось снова при обновлении
                        window.history.replaceState({}, document.title, "/login");
                    }
                };

                const timerInterval = setInterval(updateTimer, 1000);
                updateTimer(); // Вызываем сразу для отображения начального времени
            }

            // 🔐 Перехватываем отправку формы для XOR шифрования метода
            form.addEventListener('submit', async function(e) {
                e.preventDefault();
                
                // Генерируем sessionId для XOR ключа
                const sessionId = generateSessionId();
                
                // Генерируем XOR ключ (как на сервере)
                const encryptionKey = 'MT_ESP32_' + sessionId + '_METHOD_KEY';
                const limitedKey = encryptionKey.substring(0, 32);
                
                // Шифруем метод POST
                const encryptedMethod = xorEncrypt('POST', limitedKey);
                
                // 🔐 Собираем данные формы и шифруем тело
                const formData = new FormData(form);
                const loginData = {
                    username: formData.get('username'),
                    password: formData.get('password')
                };
                
                // Шифруем тело запроса
                const encryptedBody = xorEncryptBody(loginData, limitedKey);
                
                try {
                    // 🔗 URL OBFUSCATION: Применяем обфускацию URL
                    let loginURL = '/login';
                    if (window.urlObfuscationMap && window.urlObfuscationMap['/login']) {
                        loginURL = window.urlObfuscationMap['/login'];
                        console.log(`🔗 URL OBFUSCATION: /login -> ${loginURL}`);
                    }
                    
                    // Отправляем с зашифрованным методом И телом
                    const response = await fetch(loginURL, {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json',
                            'X-Real-Method': encryptedMethod,
                            'X-Client-ID': sessionId,
                            'X-Encrypted-Body': 'true'
                        },
                        body: JSON.stringify({ encrypted: encryptedBody })
                    });
                    
                    if (response.ok) {
                        // Успешный логин - редирект
                        window.location.href = '/';
                    } else {
                        // Ошибка - показываем сообщение
                        const text = await response.text();
                        if (response.status === 401) {
                            errorMessage.style.display = 'block';
                        } else if (response.status === 429) {
                            // Rate limit - показываем lockout
                            const match = text.match(/(\d+)/);
                            if (match) {
                                window.location.href = '/login?error=2&time=' + match[1];
                            }
                        } else {
                            errorMessage.style.display = 'block';
                        }
                    }
                } catch (err) {
                    console.error('Login error:', err);
                    errorMessage.textContent = '网络错误，请重试。';
                    errorMessage.style.display = 'block';
                }
            });
        });
    </script>
</body>
</html>
)rawliteral";