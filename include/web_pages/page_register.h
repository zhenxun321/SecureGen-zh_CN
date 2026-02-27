#pragma once

const char page_register[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>注册管理员 - T-Display TOTP</title>
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

        .password-criteria {
            list-style: none;
            padding: 0;
            margin: -1rem 0 1.5rem 0;
            text-align: left;
            font-size: 0.8rem;
        }

        .password-criteria li {
            color: #ff7b7b;
            margin-bottom: 0.3rem;
            transition: color 0.3s ease;
        }

        .password-criteria li.valid {
            color: #7bff9a;
        }
        

        #confirm-message {
            font-size: 0.8rem;
            margin-top: -1rem;
            margin-bottom: 1.5rem;
            height: 1rem;
        }
        .match { color: #7bff9a; }
        .no-match { color: #ff7b7b; }

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
        <h2>创建管理员账号</h2>
        <form action="/register" method="post">
            <div class="input-group">
                <label for="username">用户名</label>
                <input type="text" id="username" name="username" required autocomplete="off">
            </div>
            <div class="input-group">
                <label for="password">密码</label>
                <div class="password-input-container">
                    <input type="password" id="password" name="password" required>
                    <span class="password-toggle" onclick="togglePasswordVisibility('password', this)">👁</span>
                </div>
            </div>
            <ul class="password-criteria">
                <li id="length">至少 8 个字符</li>
                <li id="uppercase">至少 1 个大写字母</li>
                <li id="lowercase">至少 1 个小写字母</li>
                <li id="number">至少 1 个数字</li>
                <li id="special">至少 1 个特殊字符（!@#$%）</li>
            </ul>
            <div class="input-group">
                <label for="confirm-password">确认密码</label>
                <div class="password-input-container">
                    <input type="password" id="confirm-password" name="confirm_password" required>
                    <span class="password-toggle" onclick="togglePasswordVisibility('confirm-password', this)">👁</span>
                </div>
            </div>
            <div id="confirm-message"></div>
            <button type="submit" id="register-button" disabled>注册</button>
        </form>
    </div>

    <script>
        // 🔐 XOR шифрование метода для защиты Register
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

        // 🔗 Загрузка URL obfuscation mappings
        window.urlObfuscationMap = {};
        fetch('/api/url_obfuscation/mappings')
            .then(res => res.json())
            .then(data => {
                window.urlObfuscationMap = data;
                console.log('🔗 Loaded URL obfuscation mappings for register page');
            })
            .catch(err => {
                console.warn('⚠️ 加载 URL 映射失败，使用标准 URL:', err);
            });

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

        const usernameInput = document.getElementById('username');
        const passwordInput = document.getElementById('password');
        const confirmPasswordInput = document.getElementById('confirm-password');
        const registerButton = document.getElementById('register-button');
        
        const lengthCheck = document.getElementById('length');
        const uppercaseCheck = document.getElementById('uppercase');
        const lowercaseCheck = document.getElementById('lowercase');
        const numberCheck = document.getElementById('number');
        const specialCheck = document.getElementById('special');
        const confirmMessage = document.getElementById('confirm-message');

        const criteria = {
            length: { el: lengthCheck, regex: /.{8,}/ },
            uppercase: { el: uppercaseCheck, regex: /[A-Z]/ },
            lowercase: { el: lowercaseCheck, regex: /[a-z]/ },
            number: { el: numberCheck, regex: /[0-9]/ },
            special: { el: specialCheck, regex: /[!@#$%]/ }
        };

        function validatePassword() {
            const password = passwordInput.value;
            let allValid = true;
            for (const key in criteria) {
                const isValid = criteria[key].regex.test(password);
                criteria[key].el.classList.toggle('valid', isValid);
                if (!isValid) allValid = false;
            }
            return allValid;
        }

        function validateConfirmPassword() {
            const password = passwordInput.value;
            const confirmPassword = confirmPasswordInput.value;
            if (confirmPassword.length === 0) {
                confirmMessage.textContent = '';
                return false;
            }
            if (password === confirmPassword) {
                confirmMessage.textContent = '两次输入密码一致！';
                confirmMessage.className = 'match';
                return true;
            } else {
                confirmMessage.textContent = '两次输入密码不一致。';
                confirmMessage.className = 'no-match';
                return false;
            }
        }

        function checkFormValidity() {
            const isUsernameValid = usernameInput.value.length > 0;
            const isPasswordStrong = validatePassword();
            const doPasswordsMatch = validateConfirmPassword();
            
            registerButton.disabled = !(isUsernameValid && isPasswordStrong && doPasswordsMatch);
        }

        passwordInput.addEventListener('input', checkFormValidity);
        confirmPasswordInput.addEventListener('input', checkFormValidity);
        usernameInput.addEventListener('input', checkFormValidity);
        
        // Initial check
        checkFormValidity();

        // 🔐 Перехватываем отправку формы для XOR шифрования метода
        const registerForm = document.querySelector('form');
        registerForm.addEventListener('submit', async function(e) {
            e.preventDefault();
            
            // Генерируем sessionId для XOR ключа
            const sessionId = generateSessionId();
            
            // Генерируем XOR ключ (как на сервере)
            const encryptionKey = 'MT_ESP32_' + sessionId + '_METHOD_KEY';
            const limitedKey = encryptionKey.substring(0, 32);
            
            // Шифруем метод POST
            const encryptedMethod = xorEncrypt('POST', limitedKey);
            
            // 🔐 Собираем данные формы и шифруем тело
            const formData = new FormData(registerForm);
            const registerData = {
                username: formData.get('username'),
                password: formData.get('password'),
                confirm_password: formData.get('confirm_password')
            };
            
            // Шифруем тело запроса
            const encryptedBody = xorEncryptBody(registerData, limitedKey);
            
            try {
                // Отправляем с зашифрованным методом И телом
                const response = await fetch('/register', {
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
                    // Успешная регистрация - редирект на login
                    // 🔗 Используем обфусцированный URL если есть
                    let loginURL = '/login';
                    if (window.urlObfuscationMap && window.urlObfuscationMap['/login']) {
                        loginURL = window.urlObfuscationMap['/login'];
                        console.log('🔗 Register redirect to obfuscated login:', loginURL);
                    }
                    window.location.href = loginURL;
                } else {
                    // Ошибка
                    const text = await response.text();
                    alert('Registration failed: ' + text);
                }
            } catch (err) {
                console.error('Registration error:', err);
                alert('网络错误，请重试。');
            }
        });
    </script>
</body>
</html>
)rawliteral";