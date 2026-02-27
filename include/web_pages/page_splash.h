#ifndef PAGE_SPLASH_H
#define PAGE_SPLASH_H

const char page_splash_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>启动画面管理</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            max-width: 600px;
            width: 100%;
            padding: 40px;
        }
        
        h1 {
            color: #333;
            margin-bottom: 10px;
            font-size: 28px;
        }
        
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        
        .section {
            margin-bottom: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        
        .section h2 {
            color: #444;
            margin-bottom: 15px;
            font-size: 18px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .status-badge {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
        }
        
        .status-active {
            background: #d4edda;
            color: #155724;
        }
        
        .status-inactive {
            background: #f8d7da;
            color: #721c24;
        }
        
        .radio-group {
            display: flex;
            flex-direction: column;
            gap: 12px;
            margin-bottom: 15px;
        }
        
        .radio-option {
            display: flex;
            align-items: center;
            padding: 12px;
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .radio-option:hover {
            border-color: #667eea;
            background: #f8f9ff;
        }
        
        .radio-option input[type="radio"] {
            margin-right: 12px;
            width: 18px;
            height: 18px;
            cursor: pointer;
        }
        
        .radio-option label {
            cursor: pointer;
            flex: 1;
            font-weight: 500;
            color: #333;
        }
        
        .radio-option small {
            display: block;
            color: #666;
            font-size: 12px;
            margin-top: 4px;
        }
        
        .upload-area {
            border: 2px dashed #667eea;
            border-radius: 10px;
            padding: 30px;
            text-align: center;
            background: white;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .upload-area:hover {
            background: #f8f9ff;
            border-color: #764ba2;
        }
        
        .upload-area.dragover {
            background: #e8eaff;
            border-color: #667eea;
            transform: scale(1.02);
        }
        
        .upload-icon {
            font-size: 48px;
            margin-bottom: 15px;
        }
        
        input[type="file"] {
            display: none;
        }
        
        button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            width: 100%;
            margin-top: 10px;
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        button:disabled {
            background: #ccc;
            cursor: not-allowed;
            transform: none;
        }
        
        .btn-danger {
            background: linear-gradient(135deg, #f85032 0%, #e73827 100%);
        }
        
        .btn-secondary {
            background: #6c757d;
        }
        
        .message {
            padding: 12px;
            border-radius: 8px;
            margin-top: 15px;
            font-size: 14px;
            display: none;
        }
        
        .message.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        
        .message.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        
        .message.info {
            background: #d1ecf1;
            color: #0c5460;
            border: 1px solid #bee5eb;
        }
        
        .back-link {
            text-align: center;
            margin-top: 20px;
        }
        
        .back-link a {
            color: #667eea;
            text-decoration: none;
            font-weight: 600;
        }
        
        .back-link a:hover {
            text-decoration: underline;
        }
        
        .preview-info {
            background: #fff3cd;
            border: 1px solid #ffeeba;
            color: #856404;
            padding: 10px;
            border-radius: 6px;
            font-size: 13px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🖼️ 启动画面管理</h1>
        <p class="subtitle">自定义设备开机启动画面</p>
        
        <!-- Current Status -->
        <div class="section">
            <h2>📊 当前状态</h2>
            <div id="statusInfo">
                <p>模式：<span id="currentMode">加载中...</span></p>
                <p>自定义启动图：<span id="customStatus">加载中...</span></p>
            </div>
        </div>
        
        <!-- Embedded Splash Selection -->
        <div class="section">
            <h2>🎨 内置启动画面</h2>
            <div class="radio-group">
                <div class="radio-option">
                    <input type="radio" id="mode_bladerunner" name="splash_mode" value="bladerunner">
                    <label for="mode_bladerunner">
                        BladeRunner
                        <small>赛博朋克风启动画面</small>
                    </label>
                </div>
                
                <div class="radio-option">
                    <input type="radio" id="mode_combs" name="splash_mode" value="combs">
                    <label for="mode_combs">
                        Combs
                        <small>几何图案启动画面</small>
                    </label>
                </div>
                
                <div class="radio-option">
                    <input type="radio" id="mode_disabled" name="splash_mode" value="disabled">
                    <label for="mode_disabled">
                        禁用
                        <small>开机不显示启动画面</small>
                    </label>
                </div>
            </div>
            <button onclick="saveSplashMode()">💾 保存内置启动模式</button>
        </div>
        
        <!-- Custom Splash Upload -->
        <div class="section">
            <h2>📤 上传自定义启动图</h2>
            <div class="upload-area" id="uploadArea" onclick="document.getElementById('fileInput').click()">
                <div class="upload-icon">📁</div>
                <p><strong>点击选择文件</strong> 或拖拽到此处</p>
                <p style="font-size: 12px; color: #666; margin-top: 5px;">
                    上传 RGB565 原始文件（240x135，64800 字节）
                </p>
            </div>
            <input type="file" id="fileInput" accept=".raw,.bin" onchange="handleFileSelect(event)">
            
            <div class="preview-info">
                <strong>ℹ️ 如何制作自定义启动图：</strong><br>
                1. 创建一张 240x135 像素图片<br>
                2. 转换为 RGB565：<code>ffmpeg -i input.png -vf "crop=240:135,format=rgb565be" -f rawvideo output.raw</code><br>
                3. 在此上传 .raw 文件
            </div>
            
            <button id="uploadBtn" onclick="uploadSplash()" disabled>📤 上传启动图</button>
            <button class="btn-danger" onclick="deleteCustomSplash()">🗑️ 删除自定义启动图</button>
        </div>
        
        <div id="message" class="message"></div>
        
        <div class="back-link">
            <a href="/">← 返回主页面</a>
        </div>
    </div>
    
    <script>
        // 🔐 СИСТЕМА ЗАЩИЩЕННЫХ ЗАПРОСОВ - скопировано с главной страницы
        
        // CSRF Token для защиты от CSRF атак
        let csrfToken = '';

        // Получение CSRF токена для защищенных запросов
        async function fetchCsrfToken() {
            try {
                const response = await fetch('/csrf-token');
                if (response.ok) {
                    const data = await response.json();
                    csrfToken = data.token;
                    return true;
                } else if (response.status === 401) {
                    window.location.href = '/login';
                    return false;
                }
            } catch (error) {
                console.error('获取 CSRF token 失败:', error);
                return false;
            }
            return false;
        }

        // Аутентифицированный fetch с CSRF защитой
        async function makeAuthenticatedRequest(url, options = {}) {
            if (!options.headers) {
                options.headers = {};
            }
            
            // Добавляем CSRF токен в заголовки
            if (csrfToken) {
                options.headers['X-CSRF-Token'] = csrfToken;
            }
            
            // Пользовательская активность для сброса таймера сессии  
            options.headers['X-User-Activity'] = 'true';
            
            const response = await fetch(url, options);
            
            // Автоматический logout при 401
            if (response.status === 401) {
                window.location.href = '/login';
                return null;
            }
            
            return response;
        }

        // 🔐 УНИВЕРСАЛЬНАЯ ФУНКЦИЯ ДЛЯ ЗАШИФРОВАННЫХ ЗАПРОСОВ
        async function makeEncryptedRequest(url, options = {}) {
            // 🔐 КРИТИЧНО: Добавляем заголовки для активации шифрования
            if (!options.headers) {
                options.headers = {};
            }
            
            // Добавляем Client ID если secureClient готов
            if (window.secureClient && window.secureClient.isReady && window.secureClient.sessionId) {
                options.headers['X-Client-ID'] = window.secureClient.sessionId;
                options.headers['X-Secure-Request'] = 'true';
                // Шифрование будет выполнено в makeAuthenticatedRequest
            } else {
                console.warn('SecureClient 未就绪，使用普通请求');
            }
            
            // Добавляем заголовки для принудительной активации шифрования
            options.headers['X-Security-Level'] = 'secure';
            options.headers['X-User-Activity'] = 'true';
            
            const response = await makeAuthenticatedRequest(url, options);
            
            // Возвращаем Response объект для совместимости
            return response;
        }

        let selectedFile = null;
        
        // Load current status on page load
        window.addEventListener('DOMContentLoaded', async function() {
            // 🔐 Инициализация защиты: получаем CSRF токен
            const isValidSession = await fetchCsrfToken();
            if (!isValidSession) {
                return; // Перенаправлены на login
            }
            
            // Загружаем статус после успешной инициализации
            await loadStatus();
        });
        
        // Drag and drop handlers
        const uploadArea = document.getElementById('uploadArea');
        
        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.classList.add('dragover');
        });
        
        uploadArea.addEventListener('dragleave', () => {
            uploadArea.classList.remove('dragover');
        });
        
        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.classList.remove('dragover');
            
            const files = e.dataTransfer.files;
            if (files.length > 0) {
                handleFile(files[0]);
            }
        });
        
        async function loadStatus() {
            try {
                // 🔐 ЗАЩИЩЕННЫЙ ЗАПРОС - скопировано с главной страницы
                const response = await makeEncryptedRequest('/api/splash/mode');
                if (!response.ok) throw new Error('加载状态失败');
                
                const data = await response.json();
                
                // Update status display
                document.getElementById('currentMode').textContent = data.mode || '随机';
                document.getElementById('customStatus').innerHTML = data.has_custom 
                    ? '<span class="status-badge status-active">已启用</span>'
                    : '<span class="status-badge status-inactive">无</span>';
                
                // Set radio button
                const modeRadio = document.getElementById(`mode_${data.mode}`);
                if (modeRadio) {
                    modeRadio.checked = true;
                }
            } catch (error) {
                showMessage('加载状态失败：' + error.message, 'error');
            }
        }
        
        function handleFileSelect(event) {
            const file = event.target.files[0];
            handleFile(file);
        }
        
        function handleFile(file) {
            if (!file) return;
            
            selectedFile = file;
            
            // Update UI
            const uploadArea = document.getElementById('uploadArea');
            uploadArea.innerHTML = `
                <div class="upload-icon">✅</div>
                <p><strong>${file.name}</strong></p>
                <p style="font-size: 12px; color: #666;">大小：${file.size} 字节</p>
            `;
            
            document.getElementById('uploadBtn').disabled = false;
            
            // Validate file size
            if (file.size !== 64800) {
                showMessage(`警告：文件大小为 ${file.size} 字节。240x135 RGB565 图像应为 64800 字节。`, 'error');
            }
        }
        
        async function saveSplashMode() {
            const selectedMode = document.querySelector('input[name="splash_mode"]:checked');
            if (!selectedMode) {
                showMessage('请选择启动模式', 'error');
                return;
            }
            
            try {
                const formData = new FormData();
                formData.append('mode', selectedMode.value);
                
                // 🔐 ЗАЩИЩЕННЫЙ ЗАПРОС - скопировано с главной страницы
                const response = await makeEncryptedRequest('/api/splash/mode', {
                    method: 'POST',
                    body: formData
                });
                
                if (!response.ok) throw new Error('保存模式失败');
                
                showMessage('启动模式已保存！重启后生效。', 'success');
                loadStatus();
            } catch (error) {
                showMessage('保存模式失败：' + error.message, 'error');
            }
        }
        
        async function uploadSplash() {
            if (!selectedFile) {
                showMessage('请先选择文件', 'error');
                return;
            }
            
            try {
                const formData = new FormData();
                formData.append('file', selectedFile);
                
                showMessage('上传中...', 'info');
                
                const response = await fetch('/api/splash/upload', {
                    method: 'POST',
                    body: formData
                });
                
                const text = await response.text();
                
                if (!response.ok) {
                    throw new Error(text);
                }
                
                showMessage('自定义启动图上传成功！重启后可见。', 'success');
                selectedFile = null;
                document.getElementById('uploadBtn').disabled = true;
                loadStatus();
                
                // Reset upload area
                document.getElementById('uploadArea').innerHTML = `
                    <div class="upload-icon">📁</div>
                    <p><strong>点击选择文件</strong> 或拖拽到此处</p>
                    <p style="font-size: 12px; color: #666; margin-top: 5px;">
                        上传 RGB565 原始文件（240x135，64800 字节）
                    </p>
                `;
            } catch (error) {
                showMessage('上传失败：' + error.message, 'error');
            }
        }
        
        async function deleteCustomSplash() {
            if (!confirm('确定要删除自定义启动图吗？')) {
                return;
            }
            
            try {
                const response = await fetch('/api/splash/delete', {
                    method: 'POST'
                });
                
                if (!response.ok) throw new Error('删除失败');
                
                showMessage('自定义启动图已删除！', 'success');
                loadStatus();
            } catch (error) {
                showMessage('删除失败：' + error.message, 'error');
            }
        }
        
        function showMessage(text, type) {
            const messageEl = document.getElementById('message');
            messageEl.textContent = text;
            messageEl.className = `message ${type}`;
            messageEl.style.display = 'block';
            
            setTimeout(() => {
                messageEl.style.display = 'none';
            }, 5000);
        }
    </script>
</body>
</html>
)rawliteral";

#endif // PAGE_SPLASH_H
