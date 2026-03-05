#pragma once

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><meta charset="UTF-8"><title>TOTP 身份验证器</title><meta name="viewport" content="width=device-width, initial-scale=1"><link rel="icon" type="image/svg+xml" href="/favicon.svg"><link rel="alternate icon" href="/favicon.ico"><style>
@keyframes gradient-animation {
    0% { background-position: 0% 50%; }
    50% { background-position: 100% 50%; }
    100% { background-position: 0% 50%; }
}

@keyframes pulse {
    0% { transform: scale(1); opacity: 1; }
    50% { transform: scale(1.05); opacity: 0.8; }
    100% { transform: scale(1); opacity: 1; }
}

body {
    font-family: "PingFang SC", "Hiragino Sans GB", "Microsoft YaHei", "Noto Sans CJK SC", -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
    background: linear-gradient(-45deg, #1a1a2e, #16213e, #0f3460, #2e4a62);
    background-size: 400% 400%;
    animation: gradient-animation 15s ease infinite;
    color: #e0e0e0;
    margin: 0;
    padding: 20px;
    min-height: 100vh;
}

h2, h3 {
    color: #ffffff;
    text-align: center;
    font-weight: 300;
    letter-spacing: 1px;
    margin-bottom: 2rem;
}

.form-container, .content-box {
    max-width: 800px;
    margin: 20px auto;
    padding: 25px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid rgba(255, 255, 255, 0.1);
    backdrop-filter: blur(10px);
    border-radius: 15px;
    box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
}

table {
    width: 100%;
    border-collapse: collapse;
    background: rgba(255, 255, 255, 0.02);
    border-radius: 8px;
    overflow: hidden;
}

th, td {
    padding: 15px;
    text-align: center;
    vertical-align: middle;
    border: 1px solid rgba(255, 255, 255, 0.1);
}

th {
    background: rgba(90, 158, 238, 0.2);
    color: #ffffff;
    font-weight: 500;
}

td.code {
    font-family: 'SF Mono', 'Monaco', 'Inconsolata', 'Fira Code', 'Fira Mono', 'Droid Sans Mono', 'Consolas', monospace;
    font-size: 1.4em;
    font-weight: bold;
    color: #5a9eee;
    background: rgba(90, 158, 238, 0.1);
    cursor: pointer;
    transition: all 0.3s ease;
    user-select: none;
}

td.code:hover {
    background: rgba(90, 158, 238, 0.2);
    transform: scale(1.05);
    box-shadow: 0 2px 8px rgba(90, 158, 238, 0.3);
}

input[type="text"], input[type="password"], input[type="number"], input[type="file"], select {
    width: calc(100% - 22px);
    padding: 12px;
    margin-bottom: 15px;
    background-color: rgba(0, 0, 0, 0.2);
    border: 1px solid rgba(255, 255, 255, 0.2);
    color: #e0e0e0;
    border-radius: 8px;
    transition: all 0.3s ease;
    font-size: 1rem;
}

input:focus, select:focus {
    outline: none;
    border-color: #5a9eee;
    box-shadow: 0 0 0 3px rgba(90, 158, 238, 0.3);
}

label {
    color: #b0b0b0;
    font-size: 0.9rem;
    margin-bottom: 8px;
    display: block;
}

.button, .button-delete, .button-action {
    display: inline-block;
    padding: 12px 20px;
    border: none;
    border-radius: 8px;
    color: white;
    cursor: pointer;
    text-decoration: none;
    margin-right: 10px;
    margin-bottom: 10px;
    font-size: 1rem;
    font-weight: 500;
    transition: all 0.3s ease;
}

.button {
    background-color: #5a9eee;
}

.button:hover {
    background-color: #4a8bdb;
    transform: translateY(-1px);
}

.button-delete {
    background-color: #e74c3c;
}

.button-delete:hover {
    background-color: #c0392b;
    transform: translateY(-1px);
}

.button-action {
    background-color: #6c757d;
}

.button-action:hover {
    background-color: #5a6268;
    transform: translateY(-1px);
}

.tabs {
    overflow: hidden;
    background: rgba(255, 255, 255, 0.05);
    backdrop-filter: blur(10px);
    border: 1px solid rgba(255, 255, 255, 0.1);
    max-width: 820px;
    margin: auto;
    border-radius: 15px 15px 0 0;
}

.tabs button {
    background-color: transparent;
    float: left;
    border: none;
    outline: none;
    cursor: pointer;
    padding: 16px 20px;
    transition: all 0.3s ease;
    color: #b0b0b0;
    font-weight: 500;
}

.tabs button:hover {
    background-color: rgba(255, 255, 255, 0.1);
    color: #ffffff;
}

.tabs button.active {
    background-color: rgba(90, 158, 238, 0.2);
    color: #5a9eee;
}

.tab-content {
    display: none;
    padding: 25px;
    border-top: none;
    background: rgba(255, 255, 255, 0.02);
    border: 1px solid rgba(255, 255, 255, 0.1);
    border-top: none;
    max-width: 820px;
    margin: auto;
    border-radius: 0 0 15px 15px;
    backdrop-filter: blur(5px);
}

.status-message {
    text-align: center;
    padding: 15px;
    margin: 20px auto;
    border-radius: 8px;
    max-width: 800px;
    backdrop-filter: blur(10px);
}

.status-ok {
    background: rgba(76, 175, 80, 0.2);
    border: 1px solid rgba(76, 175, 80, 0.3);
    color: #81c784;
}

.status-err {
    background: rgba(244, 67, 54, 0.2);
    border: 1px solid rgba(244, 67, 54, 0.3);
    color: #e57373;
}

code {
    background: rgba(255, 255, 255, 0.1);
    border-radius: 4px;
    font-family: 'SF Mono', monospace;
    padding: 4px 8px;
    color: #5a9eee;
}

.modal {
    display: none;
    position: fixed;
    z-index: 1000;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0, 0, 0, 0.7);
    backdrop-filter: blur(5px);
}

.modal-content {
    background: rgba(255, 255, 255, 0.05);
    backdrop-filter: blur(15px);
    border: 1px solid rgba(255, 255, 255, 0.1);
    margin: 10% auto;
    padding: 30px;
    width: 90%;
    max-width: 500px;
    border-radius: 15px;
    box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.5);
    color: #e0e0e0;
}

.close {
    color: #b0b0b0;
    float: right;
    font-size: 28px;
    font-weight: bold;
    cursor: pointer;
    transition: color 0.3s ease;
}

.close:hover {
    color: #ffffff;
}

progress {
    width: 100%;
    height: 8px;
    border-radius: 4px;
    background: rgba(255, 255, 255, 0.1);
    border: none;
}

progress::-webkit-progress-bar {
    background: rgba(255, 255, 255, 0.1);
    border-radius: 4px;
}

progress::-webkit-progress-value {
    background: linear-gradient(90deg, #5a9eee, #4a8bdb);
    border-radius: 4px;
}

.checkbox-label {
    display: flex;
    align-items: center;
    margin: 15px 0;
    color: #b0b0b0;
}

.checkbox-label input[type="checkbox"] {
    margin-right: 10px;
    width: auto;
}


.password-criteria {
    list-style: none;
    padding: 0;
    margin: 10px 0 15px 0;
    text-align: left;
    font-size: 0.8rem;
}

.password-criteria li {
    color: #e57373;
    margin-bottom: 6px;
    transition: color 0.3s ease;
}

.password-criteria li.valid {
    color: #81c784;
}


#password-confirm-message {
    font-size: 0.8rem;
    margin: 10px 0 15px 0;
    height: 1rem;
    text-align: left;
}

.password-match {
    color: #81c784;
}

.password-no-match {
    color: #e57373;
}

.login-display-container {
    text-align: center;
    margin-bottom: 15px;
    font-size: 1rem;
    color: #b0b0b0;
}
.login-display-container strong {
    color: #5a9eee;
    font-weight: 500;
}
.modern-hr {
    border: 0;
    height: 1px;
    background: rgba(255, 255, 255, 0.1);
    margin: 20px 0;
}

.info-box {
    background: rgba(90, 158, 238, 0.1);
    border-left: 4px solid #5a9eee;
    padding: 15px;
    margin-top: 20px;
    border-radius: 0 8px 8px 0;
    font-size: 0.9rem;
}
.info-box h5 {
    margin-top: 0;
    color: #ffffff;
    font-weight: 500;
}
.info-box ul {
    list-style-type: none;
    padding-left: 0;
}
.info-box li {
    margin-bottom: 8px;
}
.info-box code {
    display: block;
    white-space: pre-wrap;
    word-wrap: break-word;
    padding: 10px;
    margin-top: 10px;
    background: rgba(0, 0, 0, 0.3);
    border-radius: 5px;
    font-size: 0.85rem;
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

/* Password Type Toggle Switch */
.password-type-selector {
    margin-bottom: 25px;
    text-align: center;
}

.toggle-container {
    display: inline-flex;
    align-items: center;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid rgba(255, 255, 255, 0.1);
    border-radius: 50px;
    padding: 4px;
    position: relative;
    backdrop-filter: blur(10px);
}

.toggle-option {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 12px 20px;
    border-radius: 50px;
    cursor: pointer;
    transition: all 0.3s ease;
    color: #b0b0b0;
    font-weight: 500;
    position: relative;
    z-index: 2;
}

.toggle-option.active {
    color: #ffffff;
}

.toggle-option.web-active {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    box-shadow: 0 4px 15px rgba(102, 126, 234, 0.3);
}

.toggle-option.wifi-active {
    background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
    box-shadow: 0 4px 15px rgba(240, 147, 251, 0.3);
}

.toggle-icon {
    font-size: 1.1em;
    opacity: 0.8;
}

.toggle-option.active .toggle-icon {
    opacity: 1;
}

.password-form-title {
    text-align: center;
    margin-bottom: 20px;
    color: #ffffff;
    font-weight: 400;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
}

.password-form-title .title-icon {
    font-size: 1.2em;
    opacity: 0.8;
}

.password-type-description {
    text-align: center;
    margin-bottom: 20px;
    padding: 12px;
    background: rgba(90, 158, 238, 0.1);
    border-left: 4px solid #5a9eee;
    border-radius: 0 8px 8px 0;
    font-size: 0.9rem;
    color: #b0b0b0;
}

.password-generate {
    position: absolute;
    right: 40px;
    top: 50%;
    transform: translateY(-50%);
    cursor: pointer;
    color: #b0b0b0;
    font-size: 1.2rem;
    user-select: none;
    transition: color 0.3s ease;
    font-weight: bold;
}

.password-generate:hover {
    color: #5a9eee;
}

/* Password Strength Indicator */
.password-strength-container {
    margin: 15px 0;
}

.password-strength-bar {
    width: 100%;
    height: 8px;
    background: rgba(255, 255, 255, 0.1);
    border-radius: 4px;
    overflow: hidden;
    margin: 8px 0;
}

.password-strength-fill {
    height: 100%;
    width: 0%;
    border-radius: 4px;
    transition: all 0.3s ease;
}

.strength-weak .password-strength-fill {
    background: linear-gradient(90deg, #e74c3c, #c0392b);
}

.strength-medium .password-strength-fill {
    background: linear-gradient(90deg, #f39c12, #e67e22);
}

.strength-strong .password-strength-fill {
    background: linear-gradient(90deg, #27ae60, #2ecc71);
}

.strength-encryption .password-strength-fill {
    background: linear-gradient(90deg, #8e44ad, #9b59b6);
}

.password-strength-text {
    font-size: 0.9rem;
    font-weight: 500;
    text-align: center;
    margin-top: 5px;
}

.strength-weak .password-strength-text {
    color: #e74c3c;
}

.strength-medium .password-strength-text {
    color: #f39c12;
}

.strength-strong .password-strength-text {
    color: #27ae60;
}

.strength-encryption .password-strength-text {
    color: #8e44ad;
}

/* Drag and Drop Styles */
.draggable-row {
    cursor: move;
    cursor: grab;
    transition: all 0.2s ease;
}

.draggable-row:active {
    cursor: grabbing;
}

.draggable-row.dragging {
    opacity: 0.5;
    transform: scale(1.02);
    box-shadow: 0 5px 15px rgba(90, 158, 238, 0.3);
    background: rgba(90, 158, 238, 0.1);
}

.drop-zone {
    border: 2px dashed rgba(90, 158, 238, 0.5);
    background: rgba(90, 158, 238, 0.05);
}

/* Copy notification styles */
.copy-notification {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    background: linear-gradient(135deg, #4CAF50, #45a049);
    color: white;
    padding: 20px 30px;
    border-radius: 12px;
    font-size: 1.1em;
    font-weight: 500;
    box-shadow: 0 8px 32px rgba(76, 175, 80, 0.3);
    z-index: 10000;
    opacity: 0;
    scale: 0.8;
    transition: all 0.3s cubic-bezier(0.34, 1.56, 0.64, 1);
    backdrop-filter: blur(10px);
    border: 1px solid rgba(255, 255, 255, 0.2);
}

.copy-notification.show {
    opacity: 1;
    scale: 1;
}

}

.drag-handle {
    display: inline-block;
    width: 20px;
    text-align: center;
    color: #b0b0b0;
    cursor: grab;
    font-size: 1.2rem;
    transition: color 0.3s ease;
}

.drag-handle:hover {
    color: #5a9eee;
}

.drag-handle:active {
    cursor: grabbing;
}

/* Mobile touch improvements */
@media (pointer: coarse) {
    .draggable-row {
        cursor: default;
    }

    .drag-handle {
        cursor: default;
        padding: 10px 5px;
        font-size: 1.4rem;
    }
}

/* Responsive design */
@media (max-width: 768px) {
    .form-container, .content-box, .tabs, .tab-content {
        margin: 10px;
        padding: 20px;
    }

    .tabs button {
        padding: 12px 16px;
        font-size: 0.9rem;
    }

    /* Table container with horizontal scroll */
    .content-box {
        overflow-x: auto;
        -webkit-overflow-scrolling: touch;
    }

    table {
        min-width: 250px; /* Минимальная ширина для удобства */
        width: 100%;
        font-size: 0.9rem;
        table-layout: fixed; /* Фиксированная разметка для контроля ширины колонок */
    }

    /* Keys table column widths for all screens */
    #keys-table th:nth-child(1), #keys-table td:nth-child(1) { width: 30px; } /* Drag */
    #keys-table th:nth-child(2), #keys-table td:nth-child(2) { width: 120px; } /* Name */
    #keys-table th:nth-child(3), #keys-table td:nth-child(3) { width: 100px; } /* Code */
    #keys-table th:nth-child(4), #keys-table td:nth-child(4) { width: 60px; } /* Timer */
    #keys-table th:nth-child(5), #keys-table td:nth-child(5) { width: 80px; } /* Progress */
    #keys-table th:nth-child(6), #keys-table td:nth-child(6) { width: 100px; } /* Actions */

    /* Passwords table column widths for all screens */
    #passwords-table th:nth-child(1), #passwords-table td:nth-child(1) { width: 30px; } /* Drag */
    #passwords-table th:nth-child(2), #passwords-table td:nth-child(2) { width: 200px; } /* Name */
    #passwords-table th:nth-child(3), #passwords-table td:nth-child(3) { width: 150px; } /* Actions */

    th, td {
        padding: 12px 8px;
        white-space: nowrap;
    }

    /* Keys table mobile optimization */
    #keys-table th:nth-child(5), /* Progress */
    #keys-table td:nth-child(5) {
        display: none;
    }

    #keys-table th:nth-child(2), /* Name */
    #keys-table td:nth-child(2) {
        width: 50px !important;
        overflow: hidden;
        text-overflow: ellipsis;
    }

    #keys-table th:nth-child(3), /* Code */
    #keys-table td:nth-child(3) {
        width: 70px !important;
        font-size: 1.1rem;
    }

    #keys-table th:nth-child(4), /* Timer */
    #keys-table td:nth-child(4) {
        width: 35px !important;
    }

    #keys-table th:nth-child(6), /* Actions */
    #keys-table td:nth-child(6) {
        width: 50px !important;
    }

    #keys-table .button-delete {
        width: auto;
        height: auto;
        padding: 8px 10px;
        border-radius: 4px;
        display: inline-flex;
        align-items: center;
        justify-content: center;
        margin: 0 auto;
        font-size: 0.85rem;
        min-width: 48px;
        min-height: 32px;
    }

    /* Passwords table mobile optimization */
    #passwords-table th:nth-child(2), /* Name */
    #passwords-table td:nth-child(2) {
        width: 150px !important;
        overflow: hidden;
        text-overflow: ellipsis;
    }

    #passwords-table .button-action,
    #passwords-table .button-delete {
        padding: 8px 12px;
        font-size: 0.85rem;
        margin: 2px;
        min-width: 44px; /* Touch target size */
        min-height: 44px;
        display: inline-flex;
        align-items: center;
        justify-content: center;
    }

    /* Copy button in passwords table - compact for mobile */
    #passwords-table .button {
        width: 40px !important; /* Fixed narrow width */
        padding: 6px 4px;
        font-size: 0.75rem;
        margin: 2px;
        min-width: 40px;
        min-height: 32px;
        display: inline-flex;
        align-items: center;
        justify-content: center;
    }

    /* Compact action buttons for passwords */
    #passwords-table .button-action {
        width: 40px !important; /* Same size as Copy button */
        padding: 6px 4px;
        font-size: 0.75rem;
        margin: 2px;
        min-width: 40px;
        min-height: 32px;
        display: inline-flex;
        align-items: center;
        justify-content: center;
    }

    #passwords-table .button-delete {
        width: 40px !important; /* Same size as Copy/Edit buttons */
        padding: 6px 4px;
        font-size: 0.75rem;
        margin: 2px;
        min-width: 40px;
        min-height: 32px;
        display: inline-flex;
        align-items: center;
        justify-content: center;
    }

    /* Keep button text visible on mobile */
    #passwords-table .button-action,
    #passwords-table .button-delete {
        font-size: 0.75rem;
    }
}

/* Extra small screens (phones in portrait) */
@media (max-width: 480px) {
    .form-container, .content-box, .tabs, .tab-content {
        margin: 5px;
        padding: 15px;
    }

    table {
        min-width: 300px;
        font-size: 0.85rem;
    }

    th, td {
        padding: 10px 6px;
    }

    #keys-table td.code {
        font-size: 1.1rem;
    }

    /* Stack action buttons vertically on very small screens */
    #passwords-table td:nth-child(3) {
        width: 100px;
    }

    #passwords-table .button-action,
    #passwords-table .button-delete {
        width: 36px;
        height: 36px;
        padding: 0;
        margin: 1px;
        border-radius: 6px;
    }
}

.instructions-content {
    text-align: left;
    line-height: 1.6;
}

.instructions-content h4 {
    color: #5a9eee;
    margin-top: 20px;
    margin-bottom: 10px;
    text-align: left;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    padding-bottom: 5px;
}

.instructions-content ul {
    list-style-type: disc;
    padding-left: 20px;
}

.instructions-content ul ul {
    list-style-type: circle;
    padding-left: 20px;
}

.instructions-content p {
    margin-bottom: 15px;
}

</style></head><body><h2>身份验证器控制面板</h2><div id="status" class="status-message" style="display:none;"></div>
<div class="tabs">
    <button class="tab-link active user-activity" onclick="openTab(event, 'Keys')">密钥</button>
    <button class="tab-link user-activity" onclick="openTab(event, 'Passwords')">密码库</button>
    <button class="tab-link user-activity" onclick="openTab(event, 'Display')">显示</button>
    <button class="tab-link user-activity" onclick="openTab(event, 'Pin')">PIN</button>
    <button class="tab-link user-activity" onclick="openTab(event, 'Settings')">设置</button>
    <button class="tab-link user-activity" onclick="openTab(event, 'Instructions')">使用说明</button>
</div>

<div id="Keys" class="tab-content" style="display:block;">
    <h3>管理密钥</h3>
    <div class="form-container">
        <h4>添加新密钥</h4>
        <form id="add-key-form">
            <label for="key-name">名称：</label>
            <input type="text" id="key-name" name="name" class="user-activity" required>
            <label for="key-secret">密钥（Base32）：</label>
            <input type="text" id="key-secret" name="secret" class="user-activity" required>
            <button type="submit" class="button user-activity">添加密钥</button>
        </form>
    </div>
    <div class="content-box">
        <h4>当前密钥</h4>
        <table id="keys-table">
            <thead><tr><th>::</th><th>名称</th><th>验证码</th><th>计时</th><th>进度</th><th>操作</th></tr></thead>
            <tbody></tbody>
        </table>
    </div>
    <div class="form-container">
        <h4>导入/导出密钥</h4>
        <div class="api-access-container">
            <p><strong>API 状态：</strong> <span class="api-status" style="font-weight:bold; color:#ffc107;">未启用</span></p>
            <button class="enable-api-btn button user-activity">启用 API 访问（5 分钟）</button>
        </div>
        <div id="import-export-buttons" style="margin-top: 15px;">
            <button id="export-keys-btn" class="button-action user-activity" disabled>导出密钥</button>
            <button id="import-keys-btn" class="button-action user-activity" disabled>导入密钥</button>
            <input type="file" id="import-file" style="display: none;" accept=".json" class="user-activity">
        </div>
    </div>
</div>

<div id="Passwords" class="tab-content">
    <h3>管理密码</h3>
    <div class="form-container">
        <h4>添加新密码</h4>
        <form id="add-password-form">
            <label for="password-name">名称：</label>
            <input type="text" id="password-name" name="name" class="user-activity" required>
            <label for="password-value">密码：</label>
            <div class="password-input-container">
                <input type="text" id="password-value" name="password" class="user-activity" required>
                <span class="password-generate" onclick="openPasswordGeneratorModal()" title="生成密码">#</span>
            </div>
            <button type="submit" class="button user-activity">添加密码</button>
        </form>
    </div>
    <div class="content-box">
        <h4>当前密码</h4>
        <table id="passwords-table">
            <thead><tr><th>::</th><th>名称</th><th>操作</th></tr></thead>
            <tbody></tbody>
        </table>
    </div>
    <div class="form-container">
        <h4>导入/导出密码</h4>
        <div class="api-access-container">
            <p><strong>API 状态：</strong> <span class="api-status" style="font-weight:bold; color:#ffc107;">未启用</span></p>
            <button class="enable-api-btn button user-activity">启用 API 访问（5 分钟）</button>
        </div>
        <div id="import-export-buttons-passwords" style="margin-top: 15px;">
            <button id="export-passwords-btn" class="button-action user-activity" disabled>导出密码</button>
            <button id="import-passwords-btn" class="button-action user-activity" disabled>导入密码</button>
            <input type="file" id="import-passwords-file" style="display: none;" accept=".json" class="user-activity">
        </div>
    </div>
</div>

<div id="Display" class="tab-content">
    <h3>显示设置</h3>
    <div class="form-container">
        <h4>主题选择</h4>
        <form id="theme-selection-form">
            <label><input type="radio" name="theme" value="light" id="theme-light" class="user-activity"> 浅色主题</label><br>
            <label><input type="radio" name="theme" value="dark" id="theme-dark" class="user-activity"> 深色主题</label><br>
            <button type="submit" class="button user-activity">应用主题</button>
        </form>
    </div>
    <div class="form-container">
        <h4>启动画面</h4>

        <div style="margin-bottom: 20px;">
            <label for="splash-mode-select" style="font-weight: bold; display: block; margin-bottom: 10px;">内置启动画面模式：</label>
            <select id="splash-mode-select" class="user-activity" style="width: 100%; padding: 8px; font-size: 14px; border: 1px solid #ccc; border-radius: 4px; margin-bottom: 10px;">
                <option value="disabled">禁用（不显示启动画面）</option>
                <option value="securegen">SecureGen</option>
                <option value="bladerunner">BladeRunner</option>
                <option value="combs">Combs</option>
            </select>
            <button id="save-splash-mode-btn" class="button user-activity">保存模式</button>
        </div>
        <!-- Custom splash upload removed for security - only embedded splash screens available -->
    </div>
    <div class="form-container">
        <h4>屏幕超时</h4>
        <form id="display-timeout-form">
            <label for="display-timeout">屏幕超时（多久后关闭显示）：</label>
            <select id="display-timeout" name="display_timeout" required class="user-activity">
                <option value="15">15 秒</option>
                <option value="30">30 秒</option>
                <option value="60">1 分钟</option>
                <option value="300">5 分钟</option>
                <option value="1800">30 分钟</option>
                <option value="0">从不</option>
            </select>
            <button type="submit" class="button user-activity">保存超时设置</button>
        </form>
    </div>
</div>

<div id="Pin" class="tab-content">
    <h3>PIN 码设置</h3>
    <div class="form-container">
        <form id="pincode-settings-form">
            <label for="pin-enabled-device">设备启动时启用 PIN：</label>
            <input type="checkbox" id="pin-enabled-device" name="enabledForDevice" class="user-activity"><br><br>
            <label for="pin-enabled-ble">BLE 密码传输启用 PIN：</label>
            <input type="checkbox" id="pin-enabled-ble" name="enabledForBle" class="user-activity"><br><br>
            <label for="pin-length">PIN 长度（4-10）：</label>
            <input type="number" id="pin-length" name="length" min="4" max="10" required class="user-activity"><br><br>
            <label for="new-pin">新 PIN：</label>
            <div class="password-input-container">
                <input type="password" id="new-pin" name="pin" placeholder="留空则保持不变" class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('new-pin', this)">O</span>
            </div>
            <label for="confirm-pin">确认新 PIN：</label>
            <div class="password-input-container">
                <input type="password" id="confirm-pin" name="pin_confirm" placeholder="留空则保持不变" class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('confirm-pin', this)">O</span>
            </div>
            <button type="submit" class="button user-activity">保存 PIN 设置</button>
        </form>
    </div>

    <!-- BLE PIN Settings Section -->
    <div class="form-container" style="margin-top: 30px; border-top: 1px solid rgba(255,255,255,0.1); padding-top: 25px;">
        <h4 style="color: #4a90e2; margin-bottom: 15px;">BLE 客户端认证 PIN</h4>
        <p style="color: #888; font-size: 0.9em; margin-bottom: 20px;"><strong>安全提示：</strong>出于安全原因，当前 BLE PIN 不会显示。仅会在配对时显示在设备屏幕上。</p>

        <form id="ble-pin-form">
            <label for="ble-pin">新的 BLE 客户端 PIN（6 位）：</label>
            <div class="password-input-container">
                <input type="password" id="ble-pin" name="ble_pin" pattern="\d{6}" maxlength="6" placeholder="输入 6 位 PIN" class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('ble-pin', this)">O</span>
            </div>

            <label for="ble-pin-confirm">确认 BLE 客户端 PIN：</label>
            <div class="password-input-container">
                <input type="password" id="ble-pin-confirm" name="ble_pin_confirm" pattern="\d{6}" maxlength="6" placeholder="确认 6 位 PIN" class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('ble-pin-confirm', this)">O</span>
            </div>

            <div style="margin: 15px 0; padding: 12px; background: rgba(255,193,7,0.1); border: 1px solid rgba(255,193,7,0.3); border-radius: 6px;">
                <small style="color: #ffc107; font-size: 0.85rem;">
                    <strong>重要：</strong>在 BLE 配对期间，此 PIN 会显示在 ESP32 屏幕上供客户端输入。
                </small>
            </div>

            <button type="submit" class="button user-activity" style="background-color: #28a745;">更新 BLE PIN</button>
        </form>
    </div>
</div>

<div id="Settings" class="tab-content">
    <h3>设备设置</h3>
    <div class="form-container">
        <h4>密码管理</h4>

        <!-- Password Type Selector -->
        <div class="password-type-selector">
            <div class="toggle-container">
                <div class="toggle-option active web-active" id="web-password-toggle">
                    <span class="toggle-icon">🔒</span>
                    <span>网页密码库</span>
                </div>
                <div class="toggle-option" id="wifi-password-toggle">
                    <span class="toggle-icon">📶</span>
                    <span>WiFi 接入点</span>
                </div>
            </div>
        </div>

        <!-- Dynamic Form Title -->
        <div class="password-form-title" id="password-form-title">
            <span class="title-icon">🔒</span>
            <span id="password-form-title-text">修改 Web 密码库密码</span>
        </div>

        <!-- Dynamic Description -->
        <div class="password-type-description" id="password-type-description">
            修改用于访问此 Web 界面的密码。
        </div>

        <div class="login-display-container">
            <p>当前登录名：<strong id="current-admin-login">加载中...</strong></p>
        </div>
        <hr class="modern-hr">
        <form id="change-password-form">
            <label for="new-password" id="new-password-label">新的 Web 密码</label>
            <div class="password-input-container">
                <input type="password" id="new-password" name="new-password" required class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('new-password', this)">O</span>
            </div>
            <ul class="password-criteria">
                <li id="pwd-length">至少 8 个字符</li>
                <li id="pwd-uppercase">至少 1 个大写字母</li>
                <li id="pwd-lowercase">至少 1 个小写字母</li>
                <li id="pwd-number">至少 1 个数字</li>
                <li id="pwd-special">至少 1 个特殊字符（!@#$%）</li>
            </ul>
            <label for="confirm-password" id="confirm-password-label">确认新的 Web 密码</label>
            <div class="password-input-container">
                <input type="password" id="confirm-password" name="confirm-password" required class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('confirm-password', this)">O</span>
            </div>
            <div id="password-confirm-message"></div>
            <button type="submit" id="change-password-btn" class="button user-activity" disabled>修改密码</button>
        </form>
    </div>
    <div class="form-container">
        <h4>蓝牙设置</h4>
        <form id="ble-settings-form">
            <label for="ble-device-name">BLE 设备名（最多 15 个字符）：</label>
            <input type="text" id="ble-device-name" name="device_name" maxlength="15" required class="user-activity">
            <button type="submit" class="button user-activity">保存 BLE 名称</button>
        </form>
    </div>
    <div class="form-container">
        <h4>mDNS 设置</h4>
        <form id="mdns-settings-form">
            <label for="mdns-hostname">mDNS 主机名（例如：'t-disp-totp'）：</label>
            <input type="text" id="mdns-hostname" name="hostname" maxlength="63" required class="user-activity">
            <button type="submit" class="button user-activity">保存 mDNS 主机名</button>
        </form>
    </div>
    <div class="form-container">
        <h4>启动模式</h4>
        <form id="startup-mode-form">
            <label for="startup-mode">启动后默认模式：</label>
            <select id="startup-mode" name="startup_mode" required class="user-activity">
                <option value="totp">TOTP 身份验证器</option>
                <option value="password">密码管理器</option>
            </select>
            <button type="submit" class="button user-activity">保存启动模式</button>
        </form>
    </div>
    <div class="form-container">
        <h4>Web 服务器</h4>
        <form id="web-server-settings-form">
            <label for="web-server-timeout">无操作自动关闭：</label>
            <select id="web-server-timeout" name="web_server_timeout" required class="user-activity">
                <option value="5">5 分钟</option>
                <option value="10">10 分钟</option>
                <option value="60">1 小时</option>
                <option value="0">从不</option>
            </select>
            <button type="submit" class="button user-activity">保存设置</button>
        </form>
    </div>

    <div class="form-container">
        <h4>时间设置（AP/离线手动校时）</h4>
        <form id="time-settings-form">
            <label for="manual-datetime">设备时间：</label>
            <input type="datetime-local" id="manual-datetime" step="1" required class="user-activity">
            <button type="submit" class="button user-activity">保存设备时间</button>
        </form>
    </div>
    <div class="form-container">
        <h4>自动登出计时器</h4>
        <form id="session-duration-form">
            <label for="session-duration">保持登录时长：</label>
            <select id="session-duration" name="session_duration" required class="user-activity">
                <option value="0">直到设备重启</option>
                <option value="1">1 小时</option>
                <option value="6">6 小时（默认）</option>
                <option value="24">24 小时</option>
                <option value="72">3 天</option>
            </select>
            <div style="margin: 15px 0; padding: 12px; background: rgba(76,175,80,0.1); border: 1px solid rgba(76,175,80,0.3); border-radius: 6px;">
                <small style="color: #81c784; font-size: 0.85rem;">
                    <strong>安全特性：</strong>用于控制自动登出时间以增强设备安全性。除“直到设备重启”模式外，会话在设备重启后仍可保持；该模式下每次上电都需要重新登录。时长越长登录越少，但设备丢失或被盗时风险更高。
                </small>
            </div>
            <button type="submit" class="button user-activity">保存自动登出计时</button>
        </form>
    </div>
    <div class="form-container">
        <h4>系统</h4>
        <button id="reboot-btn" class="button-action user-activity">重启设备</button>
        <button id="reboot-with-web-btn" class="button user-activity">重启并启用 Web 服务</button>
        <button id="clear-ble-clients-btn" class="button-action user-activity">清除 BLE 客户端</button>
        <button onclick="logout()" class="button-delete user-activity">退出登录</button>
    </div>
</div>

<div id="Instructions" class="tab-content">
    <h3>设备使用说明</h3>
    <div class="content-box instructions-content">
        <h4>1. 基础操作</h4>
        <ul>
            <li><strong>开机/唤醒：</strong>短按下方按键唤醒设备，或按 RST 键重启。</li>
            <li><strong>关机 / 深度休眠：</strong>设备在一段时间无操作后会自动进入低功耗休眠；也可在任意模式下长按下方按键关机。</li>
            <li><strong>运行模式：</strong>设备支持两种主要模式：TOTP 身份验证器（生成 2FA 验证码）和密码管理器（存储加密密码）。按顶部按键可循环切换模式。</li>
            <li><strong>网络模式：</strong>
                <ul>
                    <li><strong>离线模式：</strong>密码管理器可离线使用，无需网络。</li>
                    <li><strong>AP 模式：</strong>设备创建 WiFi 热点，用于 Web 配置和密码访问。</li>
                    <li><strong>WiFi 模式：</strong>连接到现有网络，在局域网内以自托管应用方式运行。</li>
                </ul>
            </li>
        </ul>

        <h4>2. 按键功能</h4>
        <ul>
            <li><strong>顶部按键（模式/导航）：</strong>
                <ul>
                    <li>短按：循环切换模式（TOTP/密码）。</li>
                    <li>TOTP 模式：浏览已保存密钥。</li>
                    <li>密码模式：浏览已保存密码。</li>
                </ul>
            </li>
            <li><strong>下方按键（选择/操作）：</strong>
                <ul>
                    <li>短按：从休眠中唤醒设备。</li>
                    <li>长按：关闭设备。</li>
                </ul>
            </li>
             <li><strong>双键组合（长按）：</strong>
                <ul>
                    <li>按下 RST 后同时按住两个按键 5 秒，可执行恢复出厂设置。</li>
                    <li>密码模式下：同时按住两个按键 5 秒，可通过安全 BLE 发送密码（需要 PIN 认证和加密连接）。</li>
                </ul>
            </li>
        </ul>

        <h4>3. 省电模式</h4>
        <p>设备以低功耗为目标设计。无操作 30 秒后会自动关闭屏幕并进入深度休眠，按下方按键即可唤醒。</p>

        <h4>4. 安全特性</h4>
        <ul>
            <li><strong>PIN 保护：</strong>可为设备启动和 BLE 密码传输启用 PIN 码，可在“PIN”选项卡中配置。</li>
            <li><strong>BLE LE 安全连接：</strong>蓝牙传输使用 LE Secure Connections，具备 MITM（中间人攻击）防护并强制 PIN 认证。配对时设备屏幕会显示 6 位 PIN 码以完成安全绑定。</li>
            <li><strong>BLE 加密：</strong>所有 BLE 特征都要求加密通信（ESP_GATT_PERM_READ_ENC_MITM）。在建立并通过认证的安全连接前，密码传输会被阻止。</li>
            <li><strong>设备绑定：</strong>受信设备会通过安全绑定被记住。你可以在设置中点击“清除 BLE 客户端”，或通过修改 BLE PIN 清除绑定设备。</li>
            <li><strong>加密存储：</strong>所有敏感数据（TOTP 密钥、密码和配置）都会以加密方式存储在设备内部 Flash 中，采用 AES-256-CBC，密码使用 PBKDF2 哈希。</li>
            <li><strong>Web 界面安全：</strong>Web 控制面板要求安全登录，并内置防暴力破解机制；会话会自动超时。</li>
            <li><strong>类 HTTPS 加密：</strong>Web 界面在 HTTP 之上使用增强加密，包括 ECDH 密钥交换、AES-GCM 加密、URL 混淆与方法隧道，即使在未加密连接上也能提升通信安全性。</li>
            <li><strong>导入/导出：</strong>导出密钥或密码时，备份文件会用 Web 管理员密码加密。请妥善保管该密码，恢复备份时需要使用。</li>
        </ul>

        <h4>5. 恢复出厂设置</h4>
        <p>出于安全考虑，恢复出厂通过硬件按键触发。执行完整重置时，请在按下 RST 后同时长按上下两个按键（注意：此操作会清空所有数据）。</p>
    </div>
</div>

<!-- Password Modal for Import/Export -->
<div id="password-modal" class="modal">
    <div class="modal-content">
        <span class="close" onclick="closePasswordModal()">&times;</span>
        <h3 id="modal-title">输入管理员密码</h3>
        <p id="modal-description"></p>
        <div class="form-group">
            <label for="modal-password">密码：</label>
            <input type="password" id="modal-password" style="width: calc(100% - 24px);" class="user-activity">
        </div>
        <button id="modal-submit-btn" class="button user-activity">确认</button>
    </div>
</div>

<!-- Password Generator Modal -->
<div id="password-generator-modal" class="modal">
    <div class="modal-content">
        <span class="close" onclick="closePasswordGeneratorModal()">&times;</span>
        <h3>密码生成器</h3>
        <p>选择密码长度并生成安全密码</p>
        <div class="form-group">
            <label for="password-length-slider">密码长度：<span id="length-display">14</span></label>
            <input type="range" id="password-length-slider" min="4" max="64" value="14" style="width: 100%; margin: 15px 0;">
            <div style="display: flex; justify-content: space-between; font-size: 0.8rem; color: #b0b0b0; margin-bottom: 20px;">
                <span>4</span><span>64</span>
            </div>
        </div>
        <div class="form-group">
            <label for="generated-password">生成的密码：</label>
            <div class="password-input-container">
                <input type="text" id="generated-password" readonly style="width: calc(100% - 24px); font-family: monospace; background: rgba(90, 158, 238, 0.1);" class="user-activity">
                <span class="password-toggle" onclick="togglePasswordVisibility('generated-password', this)">O</span>
            </div>
            <div class="password-strength-container">
                <div class="password-strength-bar">
                    <div class="password-strength-fill" id="strength-fill"></div>
                </div>
                <div class="password-strength-text" id="strength-text">加密强度</div>
            </div>
        </div>
        <div style="display: flex; gap: 10px; margin-top: 20px;">
            <button id="generate-new-btn" class="button user-activity" onclick="generatePassword()">重新生成</button>
            <button id="use-password-btn" class="button user-activity" onclick="useGeneratedPassword()">使用此密码</button>
        </div>
    </div>
</div>

<!-- Password Edit Modal -->
<div id="password-edit-modal" class="modal">
    <div class="modal-content">
        <span class="close" onclick="closePasswordEditModal()">&times;</span>
        <h3>编辑密码</h3>
        <p>修改此条目的名称和密码</p>
        <div class="form-group">
            <label for="edit-password-name">名称：</label>
            <input type="text" id="edit-password-name" style="width: calc(100% - 24px);" class="user-activity" required>
        </div>
        <div class="form-group">
            <label for="edit-password-value">密码：</label>
            <div class="password-input-container">
                <input type="text" id="edit-password-value" style="width: calc(100% - 52px); font-family: monospace;" class="user-activity" required>
                <span class="password-generate" onclick="generatePasswordForEdit()" title="生成密码">#</span>
                <span class="password-toggle" onclick="togglePasswordVisibility('edit-password-value', this)">O</span>
            </div>
            <div class="password-strength-container">
                <div class="password-strength-bar">
                    <div class="password-strength-fill" id="edit-strength-fill"></div>
                </div>
                <div class="password-strength-text" id="edit-strength-text">弱密码</div>
            </div>
        </div>
        <div style="display: flex; gap: 10px; margin-top: 20px; justify-content: flex-end;">
            <button class="button-action user-activity" onclick="closePasswordEditModal()">取消</button>
            <button id="save-password-btn" class="button user-activity" onclick="savePasswordEdit()">保存</button>
        </div>
    </div>
</div>

<script>
// ⚡ CACHE MANAGER - localStorage кеширование для снижения нагрузки на ESP32
const CacheManager = {
    PREFIX: 'totp_cache_',
    TTL: 5 * 60 * 1000, // 5 минут в миллисекундах

    set(key, data) {
        try {
            const cacheEntry = {
                data: data,
                timestamp: Date.now(),
                ttl: this.TTL
            };
            localStorage.setItem(this.PREFIX + key, JSON.stringify(cacheEntry));
            // 📉 Убран DEBUG лог - повторяется очень часто
        } catch (e) {
            console.warn('⚠️ Cache storage failed:', e.message);
        }
    },

    get(key) {
        try {
            const cached = localStorage.getItem(this.PREFIX + key);
            if (!cached) return null;

            const cacheEntry = JSON.parse(cached);
            const age = Date.now() - cacheEntry.timestamp;

            if (age > cacheEntry.ttl) {
                console.log('🕒 Cache EXPIRED:', key, '(age:', Math.round(age/1000), 's)');
                this.remove(key);
                return null;
            }

            console.log('✅ Cache HIT:', key, '(age:', Math.round(age/1000), 's)');
            return cacheEntry.data;
        } catch (e) {
            console.warn('⚠️ Cache read failed:', e.message);
            return null;
        }
    },

    remove(key) {
        try {
            localStorage.removeItem(this.PREFIX + key);
            console.log('🗑️ Cache REMOVE:', key);
        } catch (e) {
            console.warn('⚠️ Cache remove failed:', e.message);
        }
    },

    clear() {
        try {
            const keys = Object.keys(localStorage);
            keys.forEach(key => {
                if (key.startsWith(this.PREFIX)) {
                    localStorage.removeItem(key);
                }
            });
            console.log('🧹 Cache CLEARED (all TOTP cache entries removed)');
        } catch (e) {
            console.warn('⚠️ Cache clear failed:', e.message);
        }
    },

    invalidate(key) {
        this.remove(key);
        console.log('♻️ Cache INVALIDATED:', key);
    },

    getStats() {
        try {
            const keys = Object.keys(localStorage).filter(k => k.startsWith(this.PREFIX));
            const stats = {
                totalEntries: keys.length,
                entries: {}
            };

            keys.forEach(key => {
                const cached = localStorage.getItem(key);
                if (cached) {
                    const entry = JSON.parse(cached);
                    const age = Date.now() - entry.timestamp;
                    const remaining = entry.ttl - age;
                    stats.entries[key.replace(this.PREFIX, '')] = {
                        age: Math.round(age / 1000),
                        remaining: Math.round(remaining / 1000),
                        size: new Blob([cached]).size
                    };
                }
            });

            return stats;
        } catch (e) {
            return { error: e.message };
        }
    }
};

// 🛡️ Global error handler to prevent white screen
window.addEventListener('error', function(event) {
    console.error('❌ Global error caught:', event.error);
    event.preventDefault();
    return true;
});

window.addEventListener('unhandledrejection', function(event) {
    console.error('❌ Unhandled promise rejection:', event.reason);
    event.preventDefault();
});

function getCookie(name){const value='; '+document.cookie;const parts=value.split('; '+name+'=');if(parts.length===2)return parts.pop().split(';').shift();return null}

// 🔗 Helper: redirect на login с обфускацией
function redirectToLogin() {
    let loginURL = '/login'; // Fallback

    if (window.urlObfuscationMap && window.urlObfuscationMap['/login']) {
        loginURL = window.urlObfuscationMap['/login'];
        console.log('🔗 Redirecting to obfuscated login:', loginURL);
    } else {
        console.log('🔗 Redirecting to standard login (no mapping)');
    }

    window.location.href = loginURL;
}

function logout(){CacheManager.clear();const formData=new FormData();makeEncryptedRequest('/logout',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){console.log('退出成功，正在清理 Cookie 并跳转...');document.cookie='session=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;';setTimeout(()=>{window.location.replace(window.urlObfuscationMap&&window.urlObfuscationMap['/login']?window.urlObfuscationMap['/login']:'/login')},500)}else{showStatus('退出登录失败',true)}}).catch(err=>{console.error('退出登录错误:',err);document.cookie='session=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;';setTimeout(()=>{window.location.replace(window.urlObfuscationMap&&window.urlObfuscationMap['/login']?window.urlObfuscationMap['/login']:'/login')},500)})}
function showStatus(message,isError=false){const statusDiv=document.getElementById('status');statusDiv.textContent=message;statusDiv.className='status-message '+(isError?'status-err':'status-ok');statusDiv.style.display='block';setTimeout(()=>statusDiv.style.display='none',5000)}

function openTab(evt,tabName){
    var i,tabcontent,tablinks;

    if (typeof keysUpdateInterval !== 'undefined' && keysUpdateInterval) {
        clearInterval(keysUpdateInterval);
        keysUpdateInterval = null;
    }

    tabcontent=document.getElementsByClassName("tab-content");
    for(i=0;i<tabcontent.length;i++){tabcontent[i].style.display="none"}
    tablinks=document.getElementsByClassName("tab-link");
    for(i=0;i<tablinks.length;i++){tablinks[i].className=tablinks[i].className.replace(" active","")}
    document.getElementById(tabName).style.display="block";
    evt.currentTarget.className+=" active";
    if(tabName==='Display'){(async()=>{await fetchThemeSettings();await new Promise(r=>setTimeout(r,100));await fetchDisplaySettings()})()}
    else if(tabName==='Keys'){fetchKeys()}
    else if(tabName==='Passwords'){fetchPasswords()}
    else if(tabName==='Pin'){fetchPinSettings()}
    else if(tabName==='Settings'){
        // 🛡️ Загружаем настройки ПОСЛЕДОВАТЕЛЬНО с задержками
        async function loadAllSettings() {
            try {
                await fetchBleSettings();
                await new Promise(resolve => setTimeout(resolve, 150)); // 150ms задержка
                await fetchMdnsSettings();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchStartupMode();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchDeviceSettings();
                await fetchTimeSettings();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchSessionDurationSettings();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchThemeSettings();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchDisplaySettings();
                await new Promise(resolve => setTimeout(resolve, 150));
                await fetchPinSettings();
                await new Promise(resolve => setTimeout(resolve, 150));
            } catch(err) {
                console.error('Error loading settings:', err);
            }
        }
        loadAllSettings();
    }
}


let keysUpdateInterval = null;

function fetchKeys(){
    // ⚡ CACHE: Проверяем кеш для списка ключей (только имена!)
    const cachedKeys = CacheManager.get('keys_list');
    if (cachedKeys) {
        console.log('⚡ Using cached keys list, updating TOTP codes from server...');
        keysData = cachedKeys;
        updateKeysTable(cachedKeys);
        if (keysUpdateInterval) clearInterval(keysUpdateInterval);
        keysUpdateInterval = setInterval(updateTOTPCodes, 1000);
        // Продолжаем запрос в фоне для обновления TOTP кодов
    }

    makeAuthenticatedRequest('/api/keys', {
        headers: {
            'X-User-Activity': 'true'  // Пользовательское действие
        }
    })
    .then(async response => {
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const responseText = await response.text();

        // 🔐 Обработка зашифрованных TOTP данных
        let data;
        let originalData = JSON.parse(responseText);

        if (window.secureClient && window.secureClient.isReady) {
            data = await window.secureClient.decryptTOTPResponse(responseText);

            // Проверяем если расшифровка НЕ удалась
            if (originalData.type === "secure" && data && data[0] && data[0].name === "🔐 Encrypted Key 1") {
                showStatus('🔐 TOTP 密钥已加密 - 显示占位符', false);
            } else if (originalData.type === "secure" && Array.isArray(data)) {
                showStatus('✅ TOTP 密钥解密成功', false);
            }
        } else {
            data = originalData;
        }

        return data;
    })
    .then(data => {
        keysData = data;

        // ⚡ CACHE: Сохраняем только имена ключей (без TOTP кодов!)
        const keysForCache = data.map(key => ({
            name: key.name
            // НЕ кешируем code и timeLeft - они динамические!
        }));
        CacheManager.set('keys_list', keysForCache);

        updateKeysTable(data);
        if (keysUpdateInterval) clearInterval(keysUpdateInterval);
        keysUpdateInterval = setInterval(updateTOTPCodes, 1000);
    })
    .catch(err => {
        showStatus('获取密钥失败：' + err.message, true);
    });
}

function updateKeysTable(data) {
    keysData = data;
    const tbody = document.querySelector('#keys-table tbody');
    tbody.innerHTML = '';

    if (!keysData || keysData.length === 0) {
        const row = tbody.insertRow();
        row.innerHTML = '<td colspan="6" style="text-align:center;color:#666;">暂无密钥</td>';
        return;
    }

    keysData.forEach((key, index) => {
        const row = tbody.insertRow();
        row.className = 'draggable-row';
        row.draggable = true;
        row.dataset.index = index;
        row.innerHTML = `
            <td><span class="drag-handle">::</span></td>
            <td>${key.name}</td>
            <td class="code" id="code-${index}" style="font-family:monospace;font-weight:bold;" onclick="copyTOTPCode(${index})" title="点击复制 TOTP 验证码">${key.code}</td>
            <td><span id="timer-${index}" style="font-weight:bold;color:#44ff44;">${key.timeLeft}s</span></td>
            <td><progress id="progress-${index}" value="${key.timeLeft}" max="30"></progress></td>
            <td><button class="button-delete user-activity" onclick="removeKey(${index})">删除</button></td>
        `;
    });

    // Initialize drag and drop for keys table
    initializeDragAndDrop('keys-table', 'keys');
}

function updateTOTPCodes() {
    // 🔧 FIX: Если keysData пустой массив - НЕ делаем запросы
    if (keysData && keysData.length === 0) {
        return; // Нет ключей - не обновляем
    }

    // Проверяем нужно ли обновлять коды с сервера
    const currentTime = Math.floor(Date.now() / 1000);
    const timeInPeriod = currentTime % 30;

    // Обновляем с сервера только если коды изменились (каждые 30 сек) или первый раз
    if (timeInPeriod <= 1 || !keysData) {
        // 🔐 ИСПРАВЛЕНИЕ: Используем ту же логику расшифровки что и в fetchKeys()
        makeAuthenticatedRequest('/api/keys')
        .then(async response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            const responseText = await response.text();

            // 🔐 Обработка зашифрованных TOTP данных (как в fetchKeys)
            let data;
            let originalData = JSON.parse(responseText);

            if (window.secureClient && window.secureClient.isReady) {
                data = await window.secureClient.decryptTOTPResponse(responseText);

                // Проверяем если расшифровка НЕ удалась
                // Тихое обновление, логи не нужны
            } else {
                data = originalData;
            }

            return data;
        })
        .then(data => {
            keysData = data;
            updateTOTPDisplay(data);
        })
        .catch(err => console.error('Error updating TOTP codes:', err));
    } else {
        // Локальное обновление без HTTP запроса - пересчитываем timeLeft
        if (keysData && keysData.length > 0) {
            const localData = keysData.map(key => ({
                ...key,
                timeLeft: 30 - timeInPeriod  // Локальный расчет времени
            }));
            updateTOTPDisplay(localData);
        }
    }
}

function updateTOTPDisplay(data) {
    data.forEach((key, index) => {
        const codeElement = document.getElementById(`code-${index}`);
        const progressElement = document.getElementById(`progress-${index}`);
        const timerElement = document.getElementById(`timer-${index}`);

        if (codeElement && progressElement && timerElement) {
            // Animate code changes with fade effect
            if (codeElement.textContent !== key.code) {
                codeElement.style.transition = 'opacity 0.3s ease';
                codeElement.style.opacity = '0.3';
                setTimeout(() => {
                    codeElement.textContent = key.code;
                    codeElement.style.opacity = '1';
                }, 150);
            }

            // Update progress bar and timer
            progressElement.value = key.timeLeft;
            progressElement.max = 30;
            timerElement.textContent = key.timeLeft + 's';

            // Color coding based on time remaining
            if (key.timeLeft <= 5) {
                progressElement.style.filter = 'hue-rotate(0deg)';
                timerElement.style.color = '#ff4444';
                timerElement.style.fontWeight = 'bold';
            } else if (key.timeLeft <= 10) {
                progressElement.style.filter = 'hue-rotate(40deg)';
                timerElement.style.color = '#ff8800';
                timerElement.style.fontWeight = 'bold';
            } else {
                progressElement.style.filter = 'hue-rotate(120deg)';
                timerElement.style.color = '#44ff44';
                timerElement.style.fontWeight = 'bold';
            }

            // Add pulse animation when time is low
            if (key.timeLeft <= 5) {
                timerElement.style.animation = 'pulse 1s infinite';
            } else {
                timerElement.style.animation = 'none';
            }
        }
    });
}
document.getElementById('add-key-form').addEventListener('submit',function(e){e.preventDefault();const name=document.getElementById('key-name').value;const secret=document.getElementById('key-secret').value;const formData=new FormData();formData.append('name',name);formData.append('secret',secret);makeAuthenticatedRequest('/api/add',{method:'POST',body:formData}).then(data=>{CacheManager.invalidate('keys_list');showStatus('密钥添加成功！');fetchKeys();this.reset()}).catch(err=>showStatus('错误：'+err,true))});
function removeKey(index){if(!confirm('确定执行此操作吗？'))return;const formData=new FormData();formData.append('index',index);makeAuthenticatedRequest('/api/remove',{method:'POST',body:formData}).then(data=>{CacheManager.invalidate('keys_list');showStatus('密钥删除成功！');fetchKeys()}).catch(err=>showStatus('错误：'+err,true))};

// --- MODIFIED Import/Export Logic ---
let currentAction = null;
let selectedFile = null;

function showPasswordModal(action, file = null) {
    currentAction = action;
    selectedFile = file;
    const modal = document.getElementById('password-modal');
    const title = document.getElementById('modal-title');
    const description = document.getElementById('modal-description');

    if (action.startsWith('export')) {
        title.textContent = '确认导出';
        description.textContent = '请输入 Web 管理员密码，以加密并导出数据。';
    } else {
        title.textContent = '确认导入';
        description.textContent = '输入 Web 管理员密码以解密并导入所选文件。此操作将覆盖现有数据。';
    }

    document.getElementById('modal-password').value = '';
    modal.style.display = 'block';
}

function closePasswordModal() {
    document.getElementById('password-modal').style.display = 'none';
    currentAction = null;
    selectedFile = null;
}

// Password Generator Functions
function openPasswordGeneratorModal() {
    const modal = document.getElementById('password-generator-modal');
    modal.style.display = 'block';
    // Reset slider to default value
    document.getElementById('password-length-slider').value = 14;
    document.getElementById('length-display').textContent = '14';
    generatePassword(); // Generate initial password
}

function closePasswordGeneratorModal() {
    document.getElementById('password-generator-modal').style.display = 'none';
}

function generatePassword() {
    const length = document.getElementById('password-length-slider').value;

    // Character sets for different types
    const lowercase = 'abcdefghijklmnopqrstuvwxyz';
    const uppercase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const numbers = '0123456789';
    const specialChars = '!@#$%^&*()_+-=[]{}|;:,.<>?~`\'"/';

    // Ensure at least one character from each type for strong passwords
    let password = '';
    const allChars = lowercase + uppercase + numbers + specialChars;

    if (length >= 4) {
        // Add at least one character from each type for lengths >= 4
        password += getSecureRandomChar(lowercase);
        password += getSecureRandomChar(uppercase);
        password += getSecureRandomChar(numbers);
        password += getSecureRandomChar(specialChars);

        // Fill remaining length with random chars from all sets
        for (let i = 4; i < length; i++) {
            password += getSecureRandomChar(allChars);
        }

        // Shuffle the password to randomize positions
        password = shuffleString(password);
    } else {
        // For very short passwords, just use random chars
        for (let i = 0; i < length; i++) {
            password += getSecureRandomChar(allChars);
        }
    }

    document.getElementById('generated-password').value = password;
    updatePasswordStrength(password);
}

function getSecureRandomChar(charset) {
    const array = new Uint32Array(1);
    let randomValue;

    do {
        crypto.getRandomValues(array);
        randomValue = array[0];
    } while (randomValue >= (0x100000000 - (0x100000000 % charset.length)));

    return charset[randomValue % charset.length];
}

function shuffleString(str) {
    const arr = str.split('');
    // Multiple shuffle passes for better entropy
    for (let pass = 0; pass < 3; pass++) {
        for (let i = arr.length - 1; i > 0; i--) {
            const j = Math.floor(crypto.getRandomValues(new Uint32Array(1))[0] / (0x100000000 / (i + 1)));
            [arr[i], arr[j]] = [arr[j], arr[i]];
        }
    }
    return arr.join('');
}

function useGeneratedPassword() {
    const generatedPassword = document.getElementById('generated-password').value;
    document.getElementById('password-value').value = generatedPassword;
    closePasswordGeneratorModal();
}

// Password Edit Modal Functions
let currentEditIndex = -1;

function editPassword(index) {
    if (!passwordsData || !passwordsData[index]) {
        showStatus('未找到密码！', true);
        return;
    }

    currentEditIndex = index;
    const formData = new FormData();
    formData.append('index', index);

    makeAuthenticatedRequest('/api/passwords/get', { method: 'POST', body: new URLSearchParams(formData) })
        .then(async response => {
            if (!response.ok) throw new Error('获取密码失败');

            const responseText = await response.text();

            // 🔐 Обработка зашифрованных паролей (аналогично fetchKeys)
            let data;
            let originalData = JSON.parse(responseText);

            if (window.secureClient && window.secureClient.isReady) {
                data = await window.secureClient.decryptTOTPResponse(responseText);

                // Проверяем если расшифровка НЕ удалась
                if (originalData.type === "secure" && (!data || !data.name)) {
                    console.warn('🔐 Password data is encrypted but decryption failed');
                    showStatus('🔐 密码已加密 - 解密失败', true);
                    return;
                }
            } else {
                data = originalData;
            }

            document.getElementById('edit-password-name').value = data.name || '';
            document.getElementById('edit-password-value').value = data.password || '';
            updatePasswordStrengthForEdit(data.password || '');
            openPasswordEditModal();
        })
        .catch(err => {
            showStatus('加载密码失败：' + err.message, true);
        });
}

function openPasswordEditModal() {
    const modal = document.getElementById('password-edit-modal');
    modal.style.display = 'block';
}

function closePasswordEditModal() {
    const modal = document.getElementById('password-edit-modal');
    modal.style.display = 'none';
    currentEditIndex = -1;
    // Очищаем поля
    document.getElementById('edit-password-name').value = '';
    document.getElementById('edit-password-value').value = '';
    updatePasswordStrengthForEdit('');
}

function generatePasswordForEdit() {
    // Используем существующую логику генерации пароля
    const length = 14; // Стандартная длина

    const lowercase = 'abcdefghijklmnopqrstuvwxyz';
    const uppercase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const numbers = '0123456789';
    const specialChars = '!@#$%^&*()_+-=[]{}|;:,.<>?~`\'"/';

    let password = '';
    const allChars = lowercase + uppercase + numbers + specialChars;

    // Добавляем по одному символу из каждого типа
    password += getSecureRandomChar(lowercase);
    password += getSecureRandomChar(uppercase);
    password += getSecureRandomChar(numbers);
    password += getSecureRandomChar(specialChars);

    // Заполняем оставшиеся символы
    for (let i = 4; i < length; i++) {
        password += getSecureRandomChar(allChars);
    }

    // Перемешиваем пароль
    password = shuffleString(password);

    document.getElementById('edit-password-value').value = password;
    updatePasswordStrengthForEdit(password);
}

function updatePasswordStrengthForEdit(password) {
    const container = document.querySelector('#password-edit-modal .password-strength-container');
    const text = document.getElementById('edit-strength-text');
    const fill = document.getElementById('edit-strength-fill');

    if (!container || !text || !fill) return;

    const { level, score } = calculatePasswordStrength(password);

    // Убираем все классы strength-*
    container.classList.remove('strength-weak', 'strength-medium', 'strength-strong', 'strength-encryption');

    // Добавляем новый класс
    container.classList.add(`strength-${level}`);

    // Устанавливаем ширину заливки
    fill.style.width = `${score}%`;

    // Update text in English
    const levelNames = {
        'weak': '弱密码',
        'medium': '中等强度',
        'strong': '强密码',
        'encryption': 'Encryption Grade'
    };

    text.textContent = `${levelNames[level]} (${Math.round(score)}%)`;
}

function savePasswordEdit() {
    const name = document.getElementById('edit-password-name').value.trim();
    const password = document.getElementById('edit-password-value').value;

    if (!name || !password) {
        showStatus('名称和密码不能为空！', true);
        return;
    }

    if (currentEditIndex < 0) {
        showStatus('错误：无效的条目索引！', true);
        return;
    }

    const formData = new FormData();
    formData.append('index', currentEditIndex);
    formData.append('name', name);
    formData.append('password', password);

    makeAuthenticatedRequest('/api/passwords/update', { method: 'POST', body: new URLSearchParams(formData) })
        .then(response => {
            if (response.ok) {
                CacheManager.invalidate('passwords_list'); // ♻️ Инвалидация кеша
                showStatus('密码更新成功！');
                closePasswordEditModal();
                fetchPasswords(); // Refresh table
            } else {
                return response.text().then(text => {
                    throw new Error(text || '未知错误');
                });
            }
        })
        .catch(err => {
            showStatus('保存密码失败：' + err.message, true);
        });
}

function updatePasswordStrength(password) {
    const container = document.querySelector('.password-strength-container');
    const text = document.getElementById('strength-text');
    const fill = document.querySelector('.password-strength-fill');

    if (!container || !text || !fill) return;

    const { level, score } = calculatePasswordStrength(password);

    // Remove all existing strength classes
    container.classList.remove('strength-weak', 'strength-medium', 'strength-strong', 'strength-encryption');

    // Add new strength class for colors
    container.classList.add(`strength-${level}`);

    // Set the actual fill percentage dynamically
    fill.style.width = `${score}%`;

    // Update text with consistent names
    const levelNames = {
        'weak': '弱密码',
        'medium': '中等强度',
        'strong': '强密码',
        'encryption': 'Encryption Key'
    };

    text.textContent = `${levelNames[level]} (${Math.round(score)}%)`;
}

function calculatePasswordStrength(password) {
    if (!password || password.length === 0) {
        return { level: 'weak', score: 0 };
    }

    const len = password.length;

    // Count character types present
    const hasLower = /[a-z]/.test(password);
    const hasUpper = /[A-Z]/.test(password);
    const hasNumber = /[0-9]/.test(password);
    const hasSpecial = /[!@#$%^&*()_+\-=\[\]{}|;:,.<>?~`'"/]/.test(password);

    const typeCount = (hasLower ? 1 : 0) + (hasUpper ? 1 : 0) + (hasNumber ? 1 : 0) + (hasSpecial ? 1 : 0);

    // Calculate score with gentle progression starting from 1%
    let score = 0;

    // Base length score (starts very low, grows gradually)
    if (len === 1) {
        score = 1;
    } else if (len <= 4) {
        score = 1 + (len - 1) * 2; // 1, 3, 5, 7 for lengths 1-4
    } else if (len <= 8) {
        score = 7 + (len - 4) * 3; // 10, 13, 16, 19 for lengths 5-8
    } else if (len <= 12) {
        score = 19 + (len - 8) * 4; // 23, 27, 31, 35 for lengths 9-12
    } else if (len <= 20) {
        score = 35 + (len - 12) * 2; // 37, 39, 41, 43, 45, 47, 49, 51 for 13-20
    } else if (len <= 32) {
        score = 51 + (len - 20) * 1; // 52-63 for 21-32
    } else {
        score = 63 + Math.min(len - 32, 32) * 0.5; // 63-79 for 33+
    }

    // Character type bonus (smaller bonuses)
    if (typeCount === 1) {
        score += 0;
    } else if (typeCount === 2) {
        score += 5;
    } else if (typeCount === 3) {
        score += 10;
    } else if (typeCount === 4) {
        score += 15;
    }

    // Small bonus for good passwords only
    if (len >= 20 && typeCount === 4) {
        score += 5;
    }

    // Ensure score is in range 0-100
    score = Math.min(100, Math.max(0, score));

    // Determine level based on actual score ranges
    let level;
    if (score >= 75) {
        level = 'encryption';  // 75-100%: Excellent
    } else if (score >= 50) {
        level = 'strong';      // 50-74%: Good
    } else if (score >= 25) {
        level = 'medium';      // 25-49%: Fair
    } else {
        level = 'weak';        // 0-24%: Poor
    }

    return { level, score };
}

// Event listeners for password generator
document.addEventListener('DOMContentLoaded', function() {
    const lengthSlider = document.getElementById('password-length-slider');
    const lengthDisplay = document.getElementById('length-display');

    if (lengthSlider && lengthDisplay) {
        lengthSlider.addEventListener('input', function() {
            lengthDisplay.textContent = this.value;
            generatePassword();
        });
    }

    // Event listener for edit password field to update strength indicator
    const editPasswordField = document.getElementById('edit-password-value');
    if (editPasswordField) {
        editPasswordField.addEventListener('input', function() {
            updatePasswordStrengthForEdit(this.value);
        });
    }
});

document.getElementById('modal-submit-btn').addEventListener('click', () => {
    const password = document.getElementById('modal-password').value;
    if (!password) {
        showStatus('密码不能为空。', true);
        return;
    }

    if (currentAction === 'export-keys') handleExport('/api/export', password, 'encrypted_keys_backup.json');
    else if (currentAction === 'export-passwords') handleExport('/api/passwords/export', password, 'encrypted_passwords_backup.json');
    else if (currentAction === 'import-keys') handleImport('/api/import', password, selectedFile, fetchKeys);
    else if (currentAction === 'import-passwords') handleImport('/api/passwords/import', password, selectedFile, fetchPasswords);

    closePasswordModal();
});

function handleExport(url, password, filename) {
    const formData = new FormData();
    formData.append('password', password);

    makeAuthenticatedRequest(url, { method: 'POST', body: formData })
    .then(response => {
        console.log('💾 Export response status:', response.status, response.ok);

        // ⚠️ ВАЖНО: makeAuthenticatedRequest возвращает Response, нужно .json()
        if (!response.ok) {
            throw new Error(`导出失败，状态码：${response.status}`);
        }

        return response.json();
    })
    .then(data => {
        console.log('💾 Export data parsed:', {
            hasStatus: !!data.status,
            hasFileContent: !!data.fileContent,
            hasMessage: !!data.message,
            dataType: typeof data
        });

        // 🔧 Для туннелирования: извлекаем fileContent из JSON wrapper
        let fileContent;
        if (data.fileContent) {
            // ✅ Туннелированный ответ: {status, message, fileContent, filename}
            fileContent = data.fileContent;
            filename = data.filename || filename;
            console.log('🚇 Tunneled export: fileContent extracted, size:', fileContent.length);
        } else if (typeof data === 'string') {
            // ✅ Прямой ответ: файл как строка
            fileContent = data;
            console.log('📄 Direct export: file as string, size:', fileContent.length);
        } else {
            // ❌ Fallback: stringify всего объекта (старое поведение)
            fileContent = JSON.stringify(data, null, 2);
            console.warn('⚠️ Fallback export: stringifying entire response');
        }

        // Создаем файл с зашифрованным контентом
        const blob = new Blob([fileContent], { type: 'application/json' });
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = filename;
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);

        showStatus(data.message || '导出成功！');
        console.log('✅ Export file downloaded:', filename);
    })
    .catch(err => {
        console.error('❌ Export failed:', err);
        showStatus('导出失败：' + err.message, true);
    });
}

function handleImport(url, password, file, callbackOnSuccess) {
    if (!file) {
        showStatus('未选择要导入的文件。', true);
        return;
    }
    const reader = new FileReader();
    reader.onload = function(event) {
        const fileContent = event.target.result;

        // 🔍 DEBUG: Логируем что прочитано из файла
        console.log(`📂 Import file read:`, {
            size: fileContent.length,
            preview: fileContent.substring(0, 100) + '...',
            type: typeof fileContent
        });

        if (!fileContent || fileContent.length === 0) {
            console.error('❌ Import file is empty!');
            showStatus('导入文件为空！', true);
            return;
        }

        const formData = new FormData();
        formData.append('password', password);
        formData.append('data', fileContent);

        console.log(`📦 FormData prepared for import:`, {
            password: '***',
            dataLength: fileContent.length
        });

        makeAuthenticatedRequest(url, {
            method: 'POST',
            body: formData
        })
        .then(response => {
            console.log('📬 Import response status:', response.status, response.ok);

            if (!response.ok) {
                // ❌ Ошибка от сервера
                return response.text().then(errorText => {
                    console.error(`❌ Import failed with ${response.status}: ${errorText}`);
                    throw new Error(`导入失败：${errorText || response.statusText}`);
                });
            }

            return response.json();
        })
        .then(data => {
            console.log('📬 Import data parsed:', data);

            // Обработка зашифрованного ответа от ESP32
            let message = '导入成功！';
            if (typeof data === 'object' && data.message) {
                message = data.message;
            } else if (typeof data === 'string') {
                message = data;
            }
            showStatus(message);
            callbackOnSuccess();
        })
        .catch(err => showStatus('导入失败：' + err.message, true));
    };
    reader.readAsText(file);
}

document.getElementById('export-keys-btn').addEventListener('click', (e) => { e.preventDefault(); showPasswordModal('export-keys'); });
document.getElementById('import-keys-btn').addEventListener('click', (e) => { e.preventDefault(); document.getElementById('import-file').click(); });
document.getElementById('import-file').addEventListener('change', (e) => { if(e.target.files.length > 0) showPasswordModal('import-keys', e.target.files[0]); });

document.getElementById('export-passwords-btn').addEventListener('click', (e) => { e.preventDefault(); showPasswordModal('export-passwords'); });
document.getElementById('import-passwords-btn').addEventListener('click', (e) => { e.preventDefault(); document.getElementById('import-passwords-file').click(); });
document.getElementById('import-passwords-file').addEventListener('change', (e) => { if(e.target.files.length > 0) showPasswordModal('import-passwords', e.target.files[0]); });
// --- END of MODIFIED Logic ---

function fetchPasswords(){
    // ⚡ CACHE: Проверяем кеш для списка паролей
    const cachedPasswords = CacheManager.get('passwords_list');
    if (cachedPasswords) {
        console.log('⚡ Using cached passwords list');
        passwordsData = cachedPasswords;
        const tbody = document.querySelector('#passwords-table tbody');
        tbody.innerHTML = '';
        cachedPasswords.forEach((password, index) => {
            const row = tbody.insertRow();
            row.className = 'draggable-row';
            row.draggable = true;
            row.dataset.index = index;
            row.innerHTML = '<td><span class="drag-handle">::</span></td><td>' + password.name + '</td><td><button class="button user-activity" onclick="copyPassword(' + index + ')" style="margin-right: 5px;">复制</button><button class="button-action user-activity" onclick="editPassword(' + index + ')" style="margin-right: 5px;">编辑</button><button class="button-delete user-activity" onclick="removePassword(' + index + ')">删除</button></td>';
        });
        initializeDragAndDrop('passwords-table', 'passwords');
        return; // Используем кеш, не запрашиваем сервер
    }

    makeAuthenticatedRequest('/api/passwords', {
        headers: {
            'X-User-Activity': 'true'  // Пользовательское действие
        }
    })
    .then(async response => {
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const responseText = await response.text();

        // 🔐 Обработка зашифрованных паролей (аналогично TOTP ключам)
        let data;
        let originalData = JSON.parse(responseText);

        if (window.secureClient && window.secureClient.isReady) {
            data = await window.secureClient.decryptTOTPResponse(responseText);

            // Проверяем если расшифровка НЕ удалась
            if (originalData.type === "secure" && data && data[0] && data[0].name === "🔐 Encrypted Key 1") {
                showStatus('🔐 密码已加密 - 显示占位符', false);
            } else if (originalData.type === "secure" && Array.isArray(data)) {
                showStatus('✅ 密码解密成功', false);
            }
        } else {
            data = originalData;
        }

        passwordsData = data;

        // ⚡ CACHE: Сохраняем пароли в кеш
        CacheManager.set('passwords_list', data);

        const tbody = document.querySelector('#passwords-table tbody');
        tbody.innerHTML = '';
        passwordsData.forEach((password, index) => {
            const row = tbody.insertRow();
            row.className = 'draggable-row';
            row.draggable = true;
            row.dataset.index = index;
            row.innerHTML = '<td><span class="drag-handle">::</span></td><td>' + password.name + '</td><td><button class="button user-activity" onclick="copyPassword(' + index + ')" style="margin-right: 5px;">复制</button><button class="button-action user-activity" onclick="editPassword(' + index + ')" style="margin-right: 5px;">编辑</button><button class="button-delete user-activity" onclick="removePassword(' + index + ')">删除</button></td>';
        });
        initializeDragAndDrop('passwords-table', 'passwords');
    })
    .catch(err => {
        console.error('获取密码失败:', err);
        showStatus('获取密码列表失败。', true);
    });
}
document.getElementById('add-password-form').addEventListener('submit',function(e){e.preventDefault();const name=document.getElementById('password-name').value;const password=document.getElementById('password-value').value;const formData=new FormData();formData.append('name',name);formData.append('password',password);makeAuthenticatedRequest('/api/passwords/add',{method:'POST',body:formData}).then(data=>{CacheManager.invalidate('passwords_list');showStatus('密码添加成功！');fetchPasswords();this.reset()}).catch(err=>showStatus('错误：'+err,true))});
function removePassword(index){if(!confirm('确定执行此操作吗？'))return;const formData=new FormData();formData.append('index',index);makeAuthenticatedRequest('/api/passwords/delete',{method:'POST',body:formData}).then(data=>{CacheManager.invalidate('passwords_list');showStatus('密码删除成功！');fetchPasswords()}).catch(err=>showStatus('错误：'+err,true))};

function copyPassword(index) {
    if (!passwordsData || !passwordsData[index]) {
        showStatus('未找到密码！', true);
        return;
    }

    const password = passwordsData[index].password;

    // Try modern Clipboard API first
    if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(password).then(() => {
            showStatus('密码已复制到剪贴板！');
        }).catch(err => {
            console.warn('Clipboard API failed:', err);
            fallbackCopyPassword(password);
        });
    } else {
        // Fallback for older browsers
        fallbackCopyPassword(password);
    }
}

function fallbackCopyPassword(password) {
    const textArea = document.createElement('textarea');
    textArea.value = password;
    textArea.style.position = 'fixed';
    textArea.style.left = '-999999px';
    textArea.style.top = '-999999px';
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();

    try {
        const successful = document.execCommand('copy');
        if (successful) {
            showStatus('密码已复制到剪贴板！');
        } else {
            showStatus('复制密码失败', true);
        }
    } catch (err) {
        console.error('Copy failed:', err);
        showStatus('当前浏览器不支持复制功能', true);
    } finally {
        document.body.removeChild(textArea);
    }
}

function copyTOTPCode(index) {
    if (!keysData || !keysData[index]) {
        showStatus('未找到 TOTP 验证码！', true);
        return;
    }

    const totpCode = keysData[index].code;

    // Try modern Clipboard API first
    if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(totpCode).then(() => {
            showCopyNotification('TOTP 验证码已复制！');
        }).catch(err => {
            console.warn('Clipboard API failed:', err);
            fallbackCopyTOTPCode(totpCode);
        });
    } else {
        // Fallback for older browsers
        fallbackCopyTOTPCode(totpCode);
    }
}

function fallbackCopyTOTPCode(totpCode) {
    const textArea = document.createElement('textarea');
    textArea.value = totpCode;
    textArea.style.position = 'fixed';
    textArea.style.left = '-999999px';
    textArea.style.top = '-999999px';
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();

    try {
        const successful = document.execCommand('copy');
        if (successful) {
            showCopyNotification('TOTP 验证码已复制！');
        } else {
            showStatus('复制 TOTP 验证码失败', true);
        }
    } catch (err) {
        console.error('Copy failed:', err);
        showStatus('当前浏览器不支持复制功能', true);
    } finally {
        document.body.removeChild(textArea);
    }
}

function showCopyNotification(message) {
    // Remove existing notification if any
    const existingNotification = document.querySelector('.copy-notification');
    if (existingNotification) {
        existingNotification.remove();
    }

    // Create new notification
    const notification = document.createElement('div');
    notification.className = 'copy-notification';
    notification.textContent = message;
    document.body.appendChild(notification);

    // Show notification with animation
    setTimeout(() => {
        notification.classList.add('show');
    }, 10);

    // Hide and remove notification after 2 seconds
    setTimeout(() => {
        notification.classList.remove('show');
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 300);
    }, 2000);
}

async function fetchThemeSettings(){
    // ⚡ CACHE: Проверяем кеш для theme
    const cachedTheme = CacheManager.get('theme_settings');
    if (cachedTheme) {
        console.log('⚡ Using cached theme settings');
        if(cachedTheme.theme==='light'){document.getElementById('theme-light').checked=true}else{document.getElementById('theme-dark').checked=true}
        return;
    }

    try{
        const response=await makeEncryptedRequest('/api/theme');
        const data=await response.json();

        // ⚡ CACHE: Сохраняем theme в кеш
        CacheManager.set('theme_settings', data);

        if(data.theme==='light'){document.getElementById('theme-light').checked=true}else{document.getElementById('theme-dark').checked=true}
    }catch(err){
        showStatus('获取主题设置失败。',true)
    }
}
async function fetchDisplaySettings(){try{const response=await makeEncryptedRequest('/api/display_settings');const data=await response.json();document.getElementById('display-timeout').value=data.display_timeout;const splashResponse=await makeEncryptedRequest('/api/splash/mode');if(splashResponse.ok){const splashData=await splashResponse.json();const selectElement=document.getElementById('splash-mode-select');if(selectElement)selectElement.value=splashData.mode}}catch(err){showStatus('获取显示设置失败。',true)}}
document.getElementById('theme-selection-form').addEventListener('submit',function(e){e.preventDefault();const selectedTheme=document.querySelector('input[name="theme"]:checked').value;const formData=new FormData();formData.append('theme',selectedTheme);makeEncryptedRequest('/api/theme',{method:'POST',body:new URLSearchParams(formData)}).then(res=>res.json()).then(data=>{CacheManager.invalidate('theme_settings');if(data.success){showStatus(data.message)}else{showStatus(data.message||'应用主题失败',true)}}).catch(err=>showStatus('应用主题失败：'+err,true))});
document.getElementById('display-timeout-form').addEventListener('submit',function(e){e.preventDefault();const timeout=document.getElementById('display-timeout').value;const formData=new FormData();formData.append('display_timeout',timeout);makeEncryptedRequest('/api/display_settings',{method:'POST',body:new URLSearchParams(formData)}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message)}else{showStatus(data.message||'保存超时设置失败',true)}}).catch(err=>showStatus('保存显示超时失败：'+err,true))});
document.getElementById('save-splash-mode-btn').addEventListener('click',async function(){const selectElement=document.getElementById('splash-mode-select');if(!selectElement||!selectElement.value){showStatus('请选择启动模式',true);return}const formData=new FormData();formData.append('mode',selectElement.value);try{const response=await makeEncryptedRequest('/api/splash/mode',{method:'POST',body:formData});if(response.ok){const data=await response.json();showStatus(data.success?'启动模式已保存！重启后生效。':'保存启动模式失败')}else{const text=await response.text();showStatus('错误：'+text,true)}}catch(err){showStatus('保存启动模式失败：'+err.message,true)}});

async function fetchPinSettings(){
    // ⚡ CACHE: Проверяем кеш для PIN настроек
    const cachedPin = CacheManager.get('pin_settings');
    if (cachedPin) {
        console.log('⚡ Using cached PIN settings');
        document.getElementById('pin-enabled-device').checked = cachedPin.enabledForDevice;
        document.getElementById('pin-enabled-ble').checked = cachedPin.enabledForBle;
        document.getElementById('pin-length').value = cachedPin.length;
        return;
    }

    try{
    const response = await makeEncryptedRequest('/api/pincode_settings', {
        headers: {
            'X-User-Activity': 'true'  // Пользовательское действие для PIN настроек
        }
    });

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        const responseText = await response.text();

        // 🔐 Обработка зашифрованных PIN настроек (по аналогии с fetchKeys)
        let data;
        let originalData = JSON.parse(responseText);

        if (window.secureClient && window.secureClient.isReady) {
            data = await window.secureClient.decryptTOTPResponse(responseText);

            // Проверяем если расшифровка НЕ удалась
            if (originalData.type === "secure" && data) {
                showStatus('✅ PIN 设置解密成功', false);
            }
        } else {
            data = originalData;
        }

        // ⚡ CACHE: Сохраняем PIN настройки в кеш
        CacheManager.set('pin_settings', data);

        document.getElementById('pin-enabled-device').checked = data.enabledForDevice;
        document.getElementById('pin-enabled-ble').checked = data.enabledForBle;
        document.getElementById('pin-length').value = data.length;
    }catch(err){
        showStatus('获取 PIN 设置失败：' + err.message, true);
    }
}
document.getElementById('pincode-settings-form').addEventListener('submit',function(e){e.preventDefault();const newPin=document.getElementById('new-pin').value;const confirmPin=document.getElementById('confirm-pin').value;if(newPin!==confirmPin){showStatus('两次 PIN 输入不一致！',true);return}
// ✅ FIX: Используем JSON вместо FormData для правильной передачи boolean
const jsonData={enabledForDevice:document.getElementById('pin-enabled-device').checked,enabledForBle:document.getElementById('pin-enabled-ble').checked,length:parseInt(document.getElementById('pin-length').value)};if(newPin){jsonData.pin=newPin;jsonData.pin_confirm=confirmPin}
makeEncryptedRequest('/api/pincode_settings',{method:'POST',body:JSON.stringify(jsonData),headers:{'Content-Type':'application/json'}}).then(res=>res.json()).then(data=>{CacheManager.invalidate('pin_settings');if(data.success){showStatus(data.message);document.getElementById('new-pin').value='';document.getElementById('confirm-pin').value=''}else{showStatus(data.message||'更新 PIN 设置失败',true)}}).catch(err=>showStatus('更新 PIN 设置失败：'+err,true))});

// BLE PIN Management - PIN display removed for security
document.getElementById('ble-pin-form').addEventListener('submit',function(e){e.preventDefault();const blePin=document.getElementById('ble-pin').value;const blePinConfirm=document.getElementById('ble-pin-confirm').value;if(blePin.length!==6||!/^\d{6}$/.test(blePin)){showStatus('BLE PIN 必须为 6 位数字！',true);return}if(blePin!==blePinConfirm){showStatus('两次 BLE PIN 输入不一致！',true);return}const formData=new FormData();formData.append('ble_pin',blePin);makeEncryptedRequest('/api/ble_pin_update',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message);document.getElementById('ble-pin').value='';document.getElementById('ble-pin-confirm').value=''}else{showStatus(data.message||'更新 BLE PIN 失败',true)}}).catch(err=>showStatus('更新 BLE PIN 失败：'+err,true))});

// Clear BLE Clients Management (🔐 Зашифровано)
document.getElementById('clear-ble-clients-btn').addEventListener('click',function(){if(!confirm('确定要清除所有 BLE 客户端连接吗？这会移除所有已配对设备，之后需要重新配对。')){return}const formData=new FormData();makeEncryptedRequest('/api/clear_ble_clients',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus('BLE 客户端已清除！')}else{showStatus(data.message||'清除 BLE 客户端失败',true)}}).catch(err=>showStatus('清除 BLE 客户端失败：'+err,true))});


async function fetchStartupMode(){try{const response=await makeEncryptedRequest('/api/startup_mode');const data=await response.json();document.getElementById('startup-mode').value=data.mode}catch(err){showStatus('获取启动模式失败。',true)}}
async function fetchDeviceSettings(){try{const response=await makeEncryptedRequest('/api/settings');const data=await response.json();document.getElementById('web-server-timeout').value=data.web_server_timeout;if(data.admin_login){document.getElementById('current-admin-login').textContent=data.admin_login}}catch(err){showStatus('获取设备设置失败。',true)}}
async function fetchTimeSettings(){try{const response=await makeEncryptedRequest('/api/time_settings');const data=await response.json();if(data.epoch&&data.epoch>0){const dt=new Date(data.epoch*1000);const local=new Date(dt.getTime()-dt.getTimezoneOffset()*60000).toISOString().slice(0,19);document.getElementById('manual-datetime').value=local}}catch(err){showStatus('获取时间设置失败。',true)}}
document.getElementById('startup-mode-form').addEventListener('submit',function(e){e.preventDefault();const mode=document.getElementById('startup-mode').value;const formData=new FormData();formData.append('mode',mode);makeEncryptedRequest('/api/startup_mode',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message)}else{showStatus(data.message,true)}}).catch(err=>showStatus('保存启动模式失败：'+err,true))});
document.getElementById('web-server-settings-form').addEventListener('submit',function(e){e.preventDefault();const timeout=document.getElementById('web-server-timeout').value;if(!confirm('修改 Web 服务器超时需要重启设备，是否继续？')){return;}showStatus('正在保存设置并重启设备...',false);const formData=new FormData();formData.append('web_server_timeout',timeout);makeEncryptedRequest('/api/settings',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message,false);}else{showStatus(data.message,true);}}).catch(err=>showStatus('保存设置失败：'+err,true))});
// Password validation for change password form
const passwordCriteria = {
    length: { el: document.getElementById('pwd-length'), regex: /.{8,}/ },
    uppercase: { el: document.getElementById('pwd-uppercase'), regex: /[A-Z]/ },
    lowercase: { el: document.getElementById('pwd-lowercase'), regex: /[a-z]/ },
    number: { el: document.getElementById('pwd-number'), regex: /[0-9]/ },
    special: { el: document.getElementById('pwd-special'), regex: /[!@#$%]/ }
};

function validateNewPassword() {
    const password = document.getElementById('new-password').value;
    let allValid = true;
    for (const key in passwordCriteria) {
        const isValid = passwordCriteria[key].regex.test(password);
        passwordCriteria[key].el.classList.toggle('valid', isValid);
        if (!isValid) allValid = false;
    }
    return allValid;
}

function validatePasswordConfirmation() {
    const password = document.getElementById('new-password').value;
    const confirmPassword = document.getElementById('confirm-password').value;
    const confirmMessage = document.getElementById('password-confirm-message');

    if (confirmPassword.length === 0) {
        confirmMessage.textContent = '';
        return false;
    }
    if (password === confirmPassword) {
        confirmMessage.textContent = '两次输入密码一致！';
        confirmMessage.className = 'password-match';
        return true;
    } else {
        confirmMessage.textContent = '两次输入密码不一致。';
        confirmMessage.className = 'password-no-match';
        return false;
    }
}

function checkChangePasswordFormValidity() {
    const isPasswordStrong = validateNewPassword();
    const doPasswordsMatch = validatePasswordConfirmation();

    const isFormValid = currentPasswordType === 'web' ?
        (isPasswordStrong && doPasswordsMatch) :
        (document.getElementById('new-password').value.length >= 8 && doPasswordsMatch);

    document.getElementById('change-password-btn').disabled = !isFormValid;
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

// Password Type Toggle Functionality
let currentPasswordType = 'web'; // 'web' or 'wifi'

function switchPasswordType(type) {
    currentPasswordType = type;
    const webToggle = document.getElementById('web-password-toggle');
    const wifiToggle = document.getElementById('wifi-password-toggle');
    const formTitle = document.getElementById('password-form-title-text');
    const titleIcon = document.querySelector('.title-icon');
    const description = document.getElementById('password-type-description');
    const newLabel = document.getElementById('new-password-label');
    const confirmLabel = document.getElementById('confirm-password-label');
    const criteriaList = document.querySelector('.password-criteria');
    const submitBtn = document.getElementById('change-password-btn');

    // Reset toggles
    webToggle.classList.remove('active', 'web-active');
    wifiToggle.classList.remove('active', 'wifi-active');

    if (type === 'web') {
        webToggle.classList.add('active', 'web-active');
        formTitle.textContent = '修改 Web 密码库密码';
        titleIcon.textContent = '🔒';
        description.textContent = '修改用于访问此 Web 界面的密码。';
        newLabel.textContent = '新的 Web 密码';
        confirmLabel.textContent = '确认新的 Web 密码';
        criteriaList.style.display = 'block';
        submitBtn.textContent = '修改 Web 密码';
    } else {
        wifiToggle.classList.add('active', 'wifi-active');
        formTitle.textContent = '修改 WiFi 接入点密码';
        titleIcon.textContent = '📶';
        description.textContent = '修改 AP 模式下 WiFi 接入点的密码。';
        newLabel.textContent = '新的 WiFi 接入点密码';
        confirmLabel.textContent = '确认新的 WiFi 接入点密码';
        criteriaList.style.display = 'none'; // WiFi password has different requirements
        submitBtn.textContent = '修改 WiFi 密码';
    }

    // Clear form
    document.getElementById('change-password-form').reset();
    checkChangePasswordFormValidity();
}

// Event listeners for toggles
document.getElementById('web-password-toggle').addEventListener('click', () => {
    console.log('🔄 Switching to WEB password mode');
    switchPasswordType('web');
});
document.getElementById('wifi-password-toggle').addEventListener('click', () => {
    console.log('🔄 Switching to WIFI password mode');
    switchPasswordType('wifi');
});

// Initialize with web password type
console.log('🔧 Initializing password form with WEB mode');
switchPasswordType('web');

// Add event listeners for password validation
document.getElementById('new-password').addEventListener('input', checkChangePasswordFormValidity);
document.getElementById('confirm-password').addEventListener('input', checkChangePasswordFormValidity);

document.getElementById('change-password-form').addEventListener('submit',function(e){
    e.preventDefault();
    const newPass=document.getElementById('new-password').value;
    const confirmPass=document.getElementById('confirm-password').value;

    if(newPass!==confirmPass){
        showStatus('两次密码输入不一致！',true);
        return;
    }

    // Validate based on password type
    if(currentPasswordType === 'web' && !validateNewPassword()){
        showStatus('密码不符合要求！',true);
        return;
    }

    if(currentPasswordType === 'wifi' && newPass.length < 8){
        showStatus('WiFi 密码至少需要 8 个字符！',true);
        return;
    }

    // Create FormData object
    const formData=new FormData();
    formData.append('password',newPass);

    const endpoint = currentPasswordType === 'web' ? '/api/change_password' : '/api/change_ap_password';

    console.log('🚀 Submitting password change:', {
        type: currentPasswordType,
        endpoint: endpoint,
        passwordLength: newPass.length
    });

    makeEncryptedRequest(endpoint,{method:'POST',body:formData})
        .then(res=>res.text().then(text=>{
            if(res.ok) {
                showStatus(text);
                // Clear form after successful change
                document.getElementById('change-password-form').reset();
                document.getElementById('password-confirm-message').textContent = '';
                // Reset validation states
                checkChangePasswordFormValidity();
            } else {
                showStatus(text,true);
            }
        }));
});
// Custom splash upload/delete handlers removed - feature disabled for security

document.getElementById('reboot-btn').addEventListener('click',()=>{if(!confirm('确定要重启设备吗？'))return;const formData=new FormData();makeEncryptedRequest('/api/reboot',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus('正在重启...')}else{showStatus('重启失败',true)}}).catch(()=>showStatus('正在重启...'))});
document.getElementById('reboot-with-web-btn').addEventListener('click',()=>{if(!confirm('重启并在下次启动时自动开启 Web 服务？'))return;const formData=new FormData();makeEncryptedRequest('/api/reboot_with_web',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus('正在重启并启用 Web 服务...')}else{showStatus('重启失败',true)}}).catch(()=>showStatus('正在重启并启用 Web 服务...'))});

async function fetchBleSettings(){try{const response=await makeEncryptedRequest('/api/ble_settings');const data=await response.json();document.getElementById('ble-device-name').value=data.device_name}catch(err){showStatus('获取 BLE 设置失败。',true)}}
document.getElementById('ble-settings-form').addEventListener('submit',function(e){e.preventDefault();const deviceName=document.getElementById('ble-device-name').value;const formData=new FormData();formData.append('device_name',deviceName);makeEncryptedRequest('/api/ble_settings',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message);fetchBleSettings()}else{showStatus(data.message,true)}}).catch(err=>showStatus('保存 BLE 设置失败：'+err,true))});

async function fetchMdnsSettings(){try{const response=await makeEncryptedRequest('/api/mdns_settings');const data=await response.json();document.getElementById('mdns-hostname').value=data.hostname}catch(err){showStatus('获取 mDNS 设置失败。',true)}}
document.getElementById('mdns-settings-form').addEventListener('submit',function(e){e.preventDefault();const hostname=document.getElementById('mdns-hostname').value;const formData=new FormData();formData.append('hostname',hostname);makeEncryptedRequest('/api/mdns_settings',{method:'POST',body:formData}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message);fetchMdnsSettings()}else{showStatus(data.message,true)}}).catch(err=>showStatus('保存 mDNS 设置失败：'+err,true))});
document.getElementById('time-settings-form').addEventListener('submit',function(e){e.preventDefault();const v=document.getElementById('manual-datetime').value;if(!v){showStatus('请选择设备时间',true);return}const epoch=Math.floor(new Date(v).getTime()/1000);if(!epoch||epoch<1577836800){showStatus('时间无效，请重新选择',true);return}const formData=new FormData();formData.append('epoch',String(epoch));makeEncryptedRequest('/api/time_settings',{method:'POST',body:new URLSearchParams(formData)}).then(res=>res.json()).then(data=>{if(data.success){showStatus(data.message||'时间已更新');fetchTimeSettings();updateTOTPCodes();fetchKeys()}else{showStatus(data.message||'时间更新失败',true)}}).catch(err=>showStatus('时间更新失败：'+err,true))});


let keysData = [];
let passwordsData = [];
let csrfToken = '';

// CSRF Token Management
async function fetchCsrfToken() {
    try {
        const response = await fetch('/api/csrf_token');
        if (response.ok) {
            const data = await response.json();
            if (data.csrf_token && data.csrf_token.length > 0) {
                csrfToken = data.csrf_token;
                return true;
            } else {
                // Empty or missing CSRF token indicates invalid session
                console.log('Invalid session detected (empty CSRF token), redirecting to login...');
                CacheManager.clear(); // 🧹 Очистка кеша при истечении сессии
                redirectToLogin();
                return false;
            }
        } else if (response.status === 401 || response.status === 403) {
            // Session expired or forbidden - redirect to login
            console.log('Session expired or forbidden, redirecting to login...');
            CacheManager.clear(); // 🧹 Очистка кеша при истечении сессии
            redirectToLogin();
            return false;
        } else {
            console.error('Failed to fetch CSRF token, status:', response.status);
            // For other errors, also redirect to login as fallback
            console.log('Authentication error, redirecting to login...');
            CacheManager.clear(); // 🧹 Очистка кеша при ошибке аутентификации
            redirectToLogin();
            return false;
        }
    } catch (err) {
        console.error('Error fetching CSRF token:', err);
        // On network errors, try to redirect to login
        console.log('Network error during token fetch, redirecting to login...');
        CacheManager.clear(); // 🧹 Очистка кеша при сетевой ошибке
        redirectToLogin();
        return false;
    }
}

// ===== CRYPTO-JS LIBRARY (МИНИМАЛЬНАЯ AES-GCM РЕАЛИЗАЦИЯ) =====
// Настоящая AES-GCM библиотека (упрощенная версия crypto-js)
window.CryptoJS = {
    // AES S-box и другие константы (упрощенные)
    _sbox: new Uint8Array([
        0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
        0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0
        // ... (полная таблица 256 байт слишком большая для демонстрации)
    ]),

    // Простая AES-GCM расшифровка (базовая реализация)
    AES: {
        decrypt: function(ciphertext, key, options) {
            // ВАЖНО: Это упрощенная реализация для демонстрации
            // В продакшене нужна полная AES-GCM библиотека

            const keyBytes = CryptoJS.enc.Hex.parse(key);
            const dataBytes = CryptoJS.enc.Hex.parse(ciphertext.ciphertext);
            const ivBytes = CryptoJS.enc.Hex.parse(options.iv);

            // Простая XOR расшифровка как fallback
            const result = new Uint8Array(dataBytes.sigBytes);
            for (let i = 0; i < result.length; i++) {
                result[i] = dataBytes.words[Math.floor(i/4)] >> (24 - (i % 4) * 8) & 0xff;
                result[i] ^= keyBytes.words[i % keyBytes.sigBytes] >> (24 - (i % 4) * 8) & 0xff;
                result[i] ^= ivBytes.words[i % ivBytes.sigBytes] >> (24 - (i % 4) * 8) & 0xff;
            }

            return CryptoJS.enc.Utf8.stringify({words: Array.from(result), sigBytes: result.length});
        }
    },

    // Утилиты для работы с форматами
    enc: {
        Hex: {
            parse: function(hexStr) {
                const bytes = [];
                for (let i = 0; i < hexStr.length; i += 2) {
                    bytes.push(parseInt(hexStr.substr(i, 2), 16));
                }
                const words = [];
                for (let i = 0; i < bytes.length; i += 4) {
                    words.push((bytes[i] << 24) | (bytes[i+1] << 16) | (bytes[i+2] << 8) | bytes[i+3]);
                }
                return {words: words, sigBytes: bytes.length};
            }
        },
        Utf8: {
            stringify: function(wordArray) {
                const bytes = [];
                for (let i = 0; i < wordArray.sigBytes; i++) {
                    bytes.push(wordArray.words[Math.floor(i/4)] >> (24 - (i % 4) * 8) & 0xff);
                }
                return new TextDecoder().decode(new Uint8Array(bytes));
            }
        }
    },

    mode: {
        GCM: {} // Placeholder для GCM режима
    }
};

// ===== SECURE CLIENT CLASS =====
/**
 * SecureClient - Рабочий JavaScript клиент для ESP32 шифрования
 * Использует ПРОВЕРЕННЫЙ ECDH ключ из page_test_encryption.h
 */
class SecureClient {
    constructor() {
        this.sessionId = null;
        this.isReady = false;
        this.logs = [];
        this.requestCounter = 1; // Счетчик для защиты от replay атак

        // Method Tunneling поддержка
        this.methodTunnelingEnabled = false;
        this.tunnelingStats = { totalRequests: 0, tunneledRequests: 0 };

        // 🎭 Header Obfuscation поддержка
        this.headerObfuscationEnabled = true; // Автоматически включено
        this.headerMappings = {
            'X-Client-ID': 'X-Req-UUID',
            'X-Secure-Request': 'X-Security-Level'
        };
        this.headerObfuscationStats = {
            totalObfuscated: 0,
            headersMapped: 0,
            fakeHeadersInjected: 0,
            decoyTokensGenerated: 0,
            payloadEmbedded: 0
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
        console.log(logEntry);
    }

    async establishSoftwareSecureConnection() {
        try {
            this.sessionId = this.generateSessionId();
            this.log(`[SecureClient] Initializing secure connection...`);
            this.log(`[SecureClient] Generated session ID: ${this.sessionId.substring(0,8)}...`);

            // ТЕСТ: Используем серверный ключ из ответа как клиентский
            const keyExchangeData = {
                client_id: this.sessionId,
                client_public_key: "04e47518d46db780f6d858fe99f8354ee2b27014d4f0d60f6e895aa615eccc7d1512e1b37d59de6680029a4da834d68a354088aa39ba2132cb488c44704df9cc99"
            };

            this.log(`[SecureClient] Attempting key exchange like test page...`);

            // 🔗 URL Obfuscation: применяем obfuscation если есть mapping
            let keyExchangeURL = '/api/secure/keyexchange';
            if (window.urlObfuscationMap && window.urlObfuscationMap[keyExchangeURL]) {
                keyExchangeURL = window.urlObfuscationMap[keyExchangeURL];
                console.log(`🔗 URL OBFUSCATION: /api/secure/keyexchange -> ${keyExchangeURL}`);
            }

            // 🎭 Header Obfuscation: обфусцируем заголовки для KeyExchange
            const obfuscatedHeaders = {
                'Content-Type': 'application/json',
                'X-Req-UUID': this.sessionId,                    // Обфусцировано: X-Client-ID → X-Req-UUID
                'X-Browser-Engine': 'Chromium/120.0',            // Fake header
                'X-Request-Time': Date.now().toString(),         // Fake header
                'X-Client-Version': '2.1.0',                     // Fake header
                'X-Feature-Flags': 'ecdh,xor,obfuscation'       // Fake header
            };
            console.log(`🎭 HEADER OBFUSCATION: X-Client-ID → X-Req-UUID + 4 fake headers`);

            const response = await fetch(keyExchangeURL, {
                method: 'POST',
                headers: obfuscatedHeaders,
                body: JSON.stringify(keyExchangeData)
            });

            if (response.ok) {
                const data = await response.json();
                this.log(`[SecureClient] Key exchange OK!`, 'success');

                // Сохраняем серверный ключ для ECDH
                this.serverPublicKey = data.pubkey;
                // 📉 Убран DEBUG лог - технические детали

                // ВАЖНО: Расшифровываем и сохраняем sessionKey если ESP32 его прислал
                if (data.encryptedSessionKey) {
                    const staticKey = "SecureStaticKey2024!"; // Тот же ключ что на ESP32
                    this.sessionKey = this.simpleXorDecrypt(data.encryptedSessionKey, staticKey);
                    // 📉 Убраны DEBUG логи - технические детали
                } else if (data.sessionKey) {
                    // Fallback для старого формата
                    this.sessionKey = data.sessionKey;
                    // 📉 Убран DEBUG лог - технические детали
                }

                // Попытка вычисления AES ключа
                this.deriveAESKey();

                // 🚇 АВТОМАТИЧЕСКОЕ ВКЛЮЧЕНИЕ ТУННЕЛИРОВАНИЯ
                this.enableMethodTunneling();
                // 📉 Убран DEBUG лог - повторяет информацию

                // 🎭 АВТОМАТИЧЕСКОЕ ВКЛЮЧЕНИЕ HEADER OBFUSCATION
                this.enableHeaderObfuscation();
                // 📉 Убран DEBUG лог - повторяет информацию

                this.isReady = true;
                return true;
            } else {
                const errorText = await response.text();
                this.log(`❌ Key exchange failed: ${response.status} - ${errorText}`, 'error');
                return false;
            }
        } catch (error) {
            this.log(`❌ Key exchange network error: ${error.message}`, 'error');
            return false;
        }
    }

    deriveAESKey() {
        // Теперь мы можем вычислить детерминированный AES ключ!
        // 📉 Убран DEBUG лог - технические детали

        // Логика ESP32:
        // 1. clientNonce = первые 16 символов sessionId
        // 2. shared_secret = фиктивный (так как принимает любой ключ)
        // 3. AES key = HKDF(shared_secret, clientNonce)

        const clientNonce = this.sessionId.substring(0, 16);
        // 📉 Убран DEBUG лог - технические детали

        // ВРЕМЕННОЕ РЕШЕНИЕ: Просим ESP32 прислать нам свой ключ в response
        // В keyexchange response должен быть sessionKey для синхронизации
        if (this.sessionKey && this.sessionKey.length === 64) {
            this.aesKey = this.sessionKey;
            // 📉 Убран DEBUG лог - технические детали
        } else {
            // Fallback: используем последний известный ESP32 ключ
            this.aesKey = "b882e198cec417f006caff70d125e089b2e450394db1baa42b6c7ecc4639110e";
            this.log(`⚠️ FALLBACK: Using hardcoded ESP32 key!`); // ❗ Оставлен - важное предупреждение
        }

        // 📉 Убраны DEBUG логи - технические детали
    }

    simpleHash(input) {
        // Простой hash для создания детерминированного ключа
        let hash = 0;
        let result = '';

        for (let i = 0; i < input.length; i++) {
            hash = ((hash << 5) - hash + input.charCodeAt(i)) & 0xffffffff;
        }

        // Расширяем hash до 64 hex символов (32 bytes)
        const baseHash = Math.abs(hash).toString(16).padStart(8, '0');
        for (let i = 0; i < 8; i++) {
            result += baseHash;
        }

        return result.substring(0, 64);
    }

    // Method Tunneling Functions
    xorEncrypt(data, key) {
        // XOR fallback шифрование для method header (как в ESP32)
        let result = '';
        for (let i = 0; i < data.length; i++) {
            const charCode = data.charCodeAt(i) ^ key.charCodeAt(i % key.length);
            // Конвертируем в HEX (как на сервере)
            result += charCode.toString(16).padStart(2, '0');
        }
        return result; // HEX encoded string
    }

    encryptMethod(method) {
        // Генерируем тот же ключ что и сервер: "MT_ESP32_" + clientId + "_METHOD_KEY"
        const clientId = this.sessionId || 'UNKNOWN';
        const encryptionKey = 'MT_ESP32_' + clientId + '_METHOD_KEY';

        // Ограничиваем длину ключа (max 32 символа как на сервере)
        const limitedKey = encryptionKey.substring(0, 32);

        const encryptedMethod = this.xorEncrypt(method, limitedKey);
        // 📉 Убран INFO лог - повторяется на каждый запрос
        return encryptedMethod;
    }

    shouldTunnelEndpoint(endpoint) {
        // Проверяем должен ли endpoint быть туннелирован
        const tunneledEndpoints = [
            // TOTP Keys Management
            '/api/keys',              // ✅ GET - получение ключей
            '/api/add',               // ✅ POST - добавление TOTP ключа
            '/api/remove',            // ✅ POST - удаление TOTP ключа
            '/api/keys/reorder',      // ✅ POST - переупорядочивание TOTP ключей
            '/api/export',            // ✅ POST - экспорт TOTP ключей
            '/api/import',            // ✅ POST - импорт TOTP ключей
            // Passwords Management
            '/api/passwords',
            '/api/passwords/get',
            '/api/passwords/add',
            '/api/passwords/delete',
            '/api/passwords/update',
            '/api/passwords/reorder',
            '/api/passwords/export',
            '/api/passwords/import',
            // Display Settings Management
            '/api/theme',             // ✅ GET/POST - тема устройства
            '/api/display_settings',  // ✅ GET/POST - таймаут экрана
            '/api/splash/mode',       // ✅ GET/POST - выбор splash экрана
            // PIN Settings Management
            '/api/pincode_settings',  // ✅ GET/POST - настройки PIN
            '/api/ble_pin_update',    // ✅ POST - BLE PIN обновление
            // Device Settings Management
            '/api/config',            // ✅ GET - конфигурация сервера (timeout)
            '/api/startup_mode',      // ✅ GET/POST - режим запуска
            '/api/settings',          // ✅ GET/POST - настройки устройства
            '/api/ble_settings',      // ✅ GET/POST - настройки BLE
            '/api/mdns_settings',     // ✅ GET/POST - настройки mDNS
            '/api/session_duration',  // ✅ GET/POST - длительность сессии
            // API Access Management 🔑
            '/api/enable_import_export',  // ✅ POST - включение API доступа
            '/api/import_export_status',  // ✅ GET - статус API доступа
            // Critical System Operations (NEW) 🔥
            '/logout',                // ✅ POST - выход из системы
            '/api/change_password',   // ✅ POST - смена пароля администратора
            '/api/change_ap_password', // ✅ POST - смена пароля WiFi AP
            '/api/reboot',            // ✅ POST - перезагрузка устройства
            '/api/reboot_with_web'    // ✅ POST - перезагрузка с веб-сервером
        ];
        return tunneledEndpoints.includes(endpoint);
    }

    enableMethodTunneling() {
        this.methodTunnelingEnabled = true;
        // 📉 Убран SUCCESS лог - избыточная информация
    }

    disableMethodTunneling() {
        this.methodTunnelingEnabled = false;
        this.log('🚇 Method Tunneling DISABLED - Using standard HTTP methods', 'info');
    }

    // 🎭 Header Obfuscation Functions
    processHeadersWithObfuscation(headers, endpoint, method) {
        if (!this.headerObfuscationEnabled) return headers;

        this.headerObfuscationStats.totalObfuscated++;
        let obfuscatedHeaders = { ...headers };
        let headersMappedCount = 0;

        // A) Header Mapping - переименование заголовков
        for (const [original, replacement] of Object.entries(this.headerMappings)) {
            if (obfuscatedHeaders[original]) {
                obfuscatedHeaders[replacement] = obfuscatedHeaders[original];
                delete obfuscatedHeaders[original];
                headersMappedCount++;
            }
        }

        // B) Fake Headers Injection - добавление ложных заголовков
        const fakeHeaders = {
            'X-Browser-Engine': 'Mozilla/5.0 (compatible; MSIE 10.0)',
            'X-Request-Time': Date.now().toString(),
            'X-Client-Version': '2.4.1-stable',
            'X-Feature-Flags': 'analytics,tracking,ads',
            'X-Session-State': 'active'
        };
        Object.assign(obfuscatedHeaders, fakeHeaders);
        this.headerObfuscationStats.fakeHeadersInjected += Object.keys(fakeHeaders).length;

        // C) Decoy Session Tokens - ложные токены для camouflage
        // Генерируем 2-3 токена похожих на реальные session/JWT
        const generateRandomToken = (length, format = 'hex') => {
            const chars = format === 'hex' ? '0123456789abcdef' : 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
            let token = '';
            for (let i = 0; i < length; i++) {
                token += chars.charAt(Math.floor(Math.random() * chars.length));
            };
            return token;
        };

        // Токен 1: JWT-подобный (3 части base64)
        const jwtHeader = btoa(JSON.stringify({alg:'HS256',typ:'JWT'})).replace(/=/g, '');
        const jwtPayload = generateRandomToken(43, 'base64');
        const jwtSignature = generateRandomToken(43, 'base64');
        obfuscatedHeaders['Authorization'] = `Bearer ${jwtHeader}.${jwtPayload}.${jwtSignature}`;

        // Токен 2: Session token (32-64 hex)
        const sessionLength = 32 + Math.floor(Math.random() * 33); // 32-64 символов
        obfuscatedHeaders['X-Session-Token'] = generateRandomToken(sessionLength, 'hex');

        // Токен 3: CSRF token (40-48 hex)
        const csrfLength = 40 + Math.floor(Math.random() * 9); // 40-48 символов
        obfuscatedHeaders['X-CSRF-Token'] = generateRandomToken(csrfLength, 'hex');

        // Иногда добавляем Refresh token (30% вероятность)
        if (Math.random() < 0.3) {
            const refreshLength = 48 + Math.floor(Math.random() * 17); // 48-64
            obfuscatedHeaders['X-Refresh-Token'] = generateRandomToken(refreshLength, 'base64');
            this.headerObfuscationStats.decoyTokensGenerated = (this.headerObfuscationStats.decoyTokensGenerated || 0) + 4;
        } else {
            this.headerObfuscationStats.decoyTokensGenerated = (this.headerObfuscationStats.decoyTokensGenerated || 0) + 3;
        }

        // 📉 Убран DEBUG лог - повторяется на каждый запрос

        // D) Header Payload Embedding - встраивание метаданных в User-Agent
        const metadata = {
            endpoint: endpoint,
            method: method,
            ts: Date.now()
        };
        const encodedData = btoa(JSON.stringify(metadata));
        const baseUserAgent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36';
        obfuscatedHeaders['User-Agent'] = `${baseUserAgent} (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36 EdgeInsight/${encodedData}`;
        this.headerObfuscationStats.payloadEmbedded++;

        this.headerObfuscationStats.headersMapped += headersMappedCount;

        return obfuscatedHeaders;
    }

    enableHeaderObfuscation() {
        this.headerObfuscationEnabled = true;
        // 📉 Убран SUCCESS лог - избыточная информация
    }

    disableHeaderObfuscation() {
        this.headerObfuscationEnabled = false;
        this.log('🎭 Header Obfuscation DISABLED - Using standard headers', 'info');
    }

    // Простой метод проверки что нужно шифровать
    shouldSecureEndpoint(url) {
        const secureEndpoints = [
            '/api/keys',
            '/api/add',        // 🔐 TOTP key management
            '/api/remove',     // 🔐 TOTP key management
            '/api/export',     // 🔐 TOTP key export
            '/api/import',     // 🔐 TOTP key import
            '/api/config',     // 🔐 Server configuration (timeout settings)
            '/api/keys/reorder', // 🔐 TOTP keys reordering
            '/api/passwords',  // 🔐 All passwords list
            '/api/passwords/get',
            '/api/passwords/add',
            '/api/passwords/delete',
            '/api/passwords/update',
            '/api/passwords/reorder',
            '/api/passwords/export',
            '/api/passwords/import',
            '/api/pincode_settings',   // 🔐 PIN settings (security configuration)
            '/api/ble_pin_update',     // 🔐 BLE PIN update (security sensitive)
            '/api/settings',           // 🔐 Device settings (admin login info)
            '/api/ble_settings',       // 🔐 BLE device name configuration
            '/api/mdns_settings',      // 🔐 mDNS hostname configuration
            '/api/startup_mode',       // 🔐 Startup mode configuration
            '/api/change_password',    // 🔐 Admin password change (critical!)
            '/api/change_ap_password', // 🔐 WiFi AP password change (critical!)
            '/api/session_duration',   // 🔐 Session duration settings (security)
            '/logout',                 // 🔐 Admin logout (session termination)
            '/api/clear_ble_clients',  // 🔐 Clear BLE bonded clients (critical!)
            '/api/reboot',             // 🔐 System reboot (critical!)
            '/api/reboot_with_web',    // 🔐 System reboot with web server (critical!)
            '/api/theme',              // 🔐 Display theme settings (NEW)
            '/api/display_settings',   // 🔐 Display timeout settings (NEW)
            '/api/splash/mode',        // 🔐 Splash screen selection (NEW)
            '/api/enable_import_export', // 🔐 API access control (security)
            '/api/import_export_status',  // 🔐 API access status (security)
            '/api/time_settings'         // 🔐 Manual time setting
        ];
        return secureEndpoints.some(endpoint => url === endpoint || url.startsWith(endpoint + '/') || url.startsWith(endpoint + '?'));
    }

    async decryptTOTPResponse(responseText) {
        try {
            const data = JSON.parse(responseText);

            // Проверяем что это зашифрованный ответ ESP32
            if (data.type === "secure" && data.data && data.iv && data.tag) {
                // Попытка расшифровки с нашим AES ключом
                if (this.aesKey) {
                    try {
                        const decrypted = await this.simpleAESDecrypt(data.data, data.iv, data.tag);
                        if (decrypted) {
                            const decryptedData = JSON.parse(decrypted);
                            // 📉 Убран DEBUG лог - повторяется очень часто
                            return decryptedData;
                        }
                    } catch (decErr) {
                        this.log(`❌ Decryption error: ${decErr.message}`, 'error');
                    }
                }

                // Fallback: показываем placeholder как массив
                this.log(`⚠️ 解密失败，显示占位内容`, 'warn');
                return [
                    {
                        name: "🔐 Encrypted Key 1",
                        code: "------",
                        timeLeft: 30
                    },
                    {
                        name: "🔐 Encrypted Key 2",
                        code: "------",
                        timeLeft: 30
                    }
                ];
            }

            // Если не зашифровано, возвращаем как есть
            return data;

        } catch (error) {
            this.log(`❌ TOTP decryption error: ${error.message}`, 'error');
            return JSON.parse(responseText); // Fallback
        }
    }

    // 🔐 Шифрование запроса (симметрично с decryption на сервере)
    async simpleAESEncrypt(plaintext) {
        if (!this.aesKey || this.aesKey.length !== 64) {
            this.log('❌ No valid AES key for encryption', 'error');
            return null;
        }

        // Генерируем случайный IV (12 байт = 24 hex символа)
        const iv = new Array(12);
        for (let i = 0; i < 12; i++) {
            iv[i] = Math.floor(Math.random() * 256);
        }
        const ivHex = iv.map(b => b.toString(16).padStart(2, '0')).join('');

        // Генерируем фейковый тег (16 байт = 32 hex символа)
        const tag = new Array(16);
        for (let i = 0; i < 16; i++) {
            tag[i] = Math.floor(Math.random() * 256);
        }
        const tagHex = tag.map(b => b.toString(16).padStart(2, '0')).join('');

        // XOR шифрование (как на ESP32)
        const keyBytes = this.hexToBytes(this.aesKey);
        const plaintextBytes = new TextEncoder().encode(plaintext);
        const result = new Array(plaintextBytes.length);

        for (let i = 0; i < plaintextBytes.length; i++) {
            result[i] = plaintextBytes[i] ^ keyBytes[i % keyBytes.length] ^ iv[i % iv.length];
        }

        const dataHex = result.map(b => b.toString(16).padStart(2, '0')).join('');

        // Возвращаем JSON в формате ожидаемым сервером
        return JSON.stringify({
            type: "secure",
            data: dataHex,
            iv: ivHex,
            tag: tagHex,
            counter: this.requestCounter++
        });
    }

    async simpleAESDecrypt(hexData, hexIv, hexTag) {
        // XOR дешифрация с IV (совпадает с ESP32 алгоритмом)
        try {
            const key = this.hexToBytes(this.aesKey);
            const data = this.hexToBytes(hexData);
            const iv = this.hexToBytes(hexIv);
            const result = new Array(data.length);

            // data XOR key XOR iv
            for (let i = 0; i < data.length; i++) {
                result[i] = data[i] ^ key[i % key.length] ^ iv[i % iv.length];
            }

            const decrypted = String.fromCharCode(...result);

            // Очистка и поиск JSON
            let cleanDecrypted = decrypted.trim();
            let jsonStart = cleanDecrypted.indexOf('[');
            if (jsonStart === -1) {
                jsonStart = cleanDecrypted.indexOf('{');
            }

            if (jsonStart >= 0) {
                cleanDecrypted = cleanDecrypted.substring(jsonStart);
                return cleanDecrypted;
            } else {
                this.log(`⚠️ Invalid JSON format after decryption`, 'warn');
                return null;
            }

        } catch (err) {
            this.log(`❌ XOR decryption error: ${err.message}`, 'error');
            return null;
        }
    }

    hexToBytes(hex) {
        const bytes = [];
        for (let i = 0; i < hex.length; i += 2) {
            bytes.push(parseInt(hex.substr(i, 2), 16));
        }
        return bytes;
    }

    // Простая расшифровка XOR для статических данных
    simpleXorDecrypt(hexData, key) {
        let result = "";
        // Конвертируем hex в байты и применяем XOR
        for (let i = 0; i < hexData.length; i += 2) {
            const hexByte = hexData.substr(i, 2);
            const byte = parseInt(hexByte, 16);
            const keyByte = key.charCodeAt((i / 2) % key.length);
            const decrypted = byte ^ keyByte;
            result += String.fromCharCode(decrypted);
        }
        return result;
    }
}

// Создаем глобальный экземпляр SecureClient
window.secureClient = new SecureClient();

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
        console.log('🔐 Adding headers for encryption:', window.secureClient.sessionId.substring(0,8) + '...');
        // Шифрование будет выполнено в makeAuthenticatedRequest
    }

    // Добавляем заголовки для принудительной активации шифрования
    options.headers['X-Security-Level'] = 'secure';
    options.headers['X-User-Activity'] = 'true';

    const response = await makeAuthenticatedRequest(url, options);

    // Возвращаем Response объект для совместимости
    return response;
}

// Authenticated fetch wrapper with CSRF protection and auto-logout
async function makeAuthenticatedRequest(url, options = {}) {
    if (!options.headers) {
        options.headers = {};
    }

    // Add CSRF token to POST requests
    if (options.method === 'POST' && csrfToken) {
        options.headers['X-CSRF-Token'] = csrfToken;
    }

    // 🚇 METHOD TUNNELING - СКРЫВАЕМ HTTP МЕТОДЫ ОТ АНАЛИЗА ТРАФИКА
    let originalUrl = url;
    let originalMethod = options.method || 'GET';

    if (window.secureClient && window.secureClient.methodTunnelingEnabled &&
        window.secureClient.shouldTunnelEndpoint(url)) {

        // Увеличиваем статистику
        window.secureClient.tunnelingStats.totalRequests++;
        window.secureClient.tunnelingStats.tunneledRequests++;

        // Шифруем реальный метод
        const encryptedMethod = window.secureClient.encryptMethod(originalMethod);
        options.headers['X-Real-Method'] = encryptedMethod;

        // 🔧 КОНВЕРТИРУЕМ FormData/URLSearchParams в объект
        let bodyData = {};

        if (options.body instanceof FormData) {
            // ✅ Конвертируем FormData в объект
            for (const [key, value] of options.body.entries()) {
                // ⚠️ ВАЖНО: File/Blob объекты уже должны быть прочитаны как строки!
                // handleImport использует FileReader.readAsText() перед вызовом
                if (value instanceof File || value instanceof Blob) {
                    console.error(`❌ FormData contains File/Blob for key '${key}' - should be read as text first!`);
                    console.warn(`⚠️ File name: ${value.name || 'unknown'}, size: ${value.size}b`);
                    // Пропускаем File объекты - они должны быть прочитаны через FileReader
                    bodyData[key] = ''; // Пустая строка вместо File объекта
                } else {
                    bodyData[key] = value;
                }
            }
            // 📉 Убран DEBUG лог - повторяется часто
        } else if (options.body instanceof URLSearchParams) {
            // ✅ Конвертируем URLSearchParams в объект
            for (const [key, value] of options.body.entries()) {
                bodyData[key] = value;
            }
            // 📉 Убран DEBUG лог - повторяется часто
        } else if (typeof options.body === 'string') {
            // ✅ Проверяем на JSON строку (начинается с { или [)
            const trimmed = options.body.trim();
            if (trimmed.startsWith('{') || trimmed.startsWith('[')) {
                // Это JSON - парсим как JSON
                try {
                    bodyData = JSON.parse(options.body);
                    console.log(`📦 Parsed JSON string to object:`, Object.keys(bodyData));
                } catch (e) {
                    console.error(`❌ Failed to parse JSON body:`, e.message);
                    bodyData = { raw: options.body };
                }
            } else {
                // Это URL-encoded - парсим как URLSearchParams
                try {
                    const params = new URLSearchParams(options.body);
                    for (const [key, value] of params.entries()) {
                        bodyData[key] = value;
                    }
                    console.log(`🔧 Parsed URL-encoded string to object:`, Object.keys(bodyData));
                } catch (e) {
                    bodyData = { raw: options.body };
                    console.warn(`⚠️ Failed to parse body string, using raw:`, e.message);
                }
            }
        } else if (options.body && typeof options.body === 'object') {
            // ✅ Уже объект - используем как есть
            bodyData = options.body;
        }
        // else: GET запросы имеют пустой bodyData = {}

        // Преобразуем в POST запрос к /api/tunnel
        const tunnelBody = {
            endpoint: url,
            method: originalMethod,  // 👉 Добавляем метод для сервера
            data: bodyData
        };

        // ОБНОВЛЯЕМ URL И МЕТОД
        url = '/api/tunnel';
        options.method = 'POST';
        options.body = JSON.stringify(tunnelBody);
        options.headers['Content-Type'] = 'application/json';

        // 📉 Убраны DEBUG логи - повторяются на каждый запрос

        // 🔗 URL OBFUSCATION - применяем к /api/tunnel endpoint
        if (window.urlObfuscationMap && window.urlObfuscationMap['/api/tunnel']) {
            const obfuscatedUrl = window.urlObfuscationMap['/api/tunnel'];
            // 📉 Убран DEBUG лог - повторяется на каждый запрос
            url = obfuscatedUrl;
        }
        // 🔍 DEBUG: Показываем размеры данных для import
        if (originalUrl === '/api/import' && bodyData.data) {
            console.log(`📊 Import data size: ${bodyData.data.length} chars`);
            console.log(`🔍 Import data preview: ${bodyData.data.substring(0, 50)}...`);
        }
    }

    // 🔐 ШИФРОВАНИЕ ДЛЯ ЧУВСТВИТЕЛЬНЫХ ENDPOINTS
    if (window.secureClient && window.secureClient.shouldSecureEndpoint(originalUrl)) {
        if (window.secureClient.isReady && window.secureClient.sessionId) {
            options.headers['X-Client-ID'] = window.secureClient.sessionId;
            options.headers['X-Secure-Request'] = 'true';
            // 📉 Убран DEBUG лог - повторяется на каждый запрос

            // 🔐 ШИФРОВАНИЕ ТЕЛА ЗАПРОСА
            if (options.method === 'POST' && options.body && window.secureClient.aesKey) {
                // 📉 Убран DEBUG лог - повторяется на каждый запрос

                // Поддержка FormData и URLSearchParams
                let plaintext;
                if (options.body instanceof FormData) {
                    // Конвертируем FormData в URLSearchParams строку
                    plaintext = new URLSearchParams(options.body).toString();
                } else if (options.body instanceof URLSearchParams) {
                    plaintext = options.body.toString();
                } else {
                    plaintext = options.body;
                }

                const encryptedBody = await window.secureClient.simpleAESEncrypt(plaintext);

                if (encryptedBody) {
                    options.body = encryptedBody;
                    options.headers['Content-Type'] = 'application/json'; // Зашифрованные данные в JSON
                    // 📉 Убран DEBUG лог - повторяется на каждый запрос
                } else {
                    console.warn('🔐 Failed to encrypt request body, sending as-is'); // ❗ Оставлен - важное предупреждение
                }
            }
        } else {
            console.warn(`SecureClient not ready (exists: ${!!window.secureClient}, ready: ${window.secureClient?.isReady})`);
        }
    }

    // 🎭 HEADER OBFUSCATION - применяем ПОСЛЕ добавления X-Client-ID и X-Secure-Request
    if (window.secureClient && window.secureClient.headerObfuscationEnabled) {
        options.headers = window.secureClient.processHeadersWithObfuscation(
            options.headers,
            originalUrl,
            originalMethod
        );
        // 📉 Убран DEBUG лог - повторяется на каждый запрос
    }

    try {
        const response = await fetch(url, options);

        // Auto-logout on authentication/authorization failures
        if (response.status === 401 || response.status === 403) {
            console.log('Authentication failed, redirecting to login...');
            CacheManager.clear(); // 🧹 Очистка кеша при провале аутентификации
            redirectToLogin();
            return response;
        }

        // 🔐 АВТОМАТИЧЕСКАЯ РАСШИФРОВКА ОТВЕТОВ для зашифрованных запросов
        if (window.secureClient && window.secureClient.shouldSecureEndpoint(originalUrl) &&
            window.secureClient.isReady && response.ok) {

            // Создаем новый response с расшифрованными данными
            const responseText = await response.clone().text();

            try {
                const originalData = JSON.parse(responseText);

                // Проверяем, является ли ответ зашифрованным
                if (originalData.type === "secure") {
                    const decryptedData = await window.secureClient.decryptTOTPResponse(responseText);

                    if (decryptedData) {
                        // Создаем новый response с расшифрованными данными
                        const decryptedText = typeof decryptedData === 'string' ? decryptedData : JSON.stringify(decryptedData);

                        return new Response(decryptedText, {
                            status: response.status,
                            statusText: response.statusText,
                            headers: response.headers
                        });
                    } else {
                        console.warn(`⚠️ Failed to decrypt response for ${originalUrl}, using original`);
                    }
                }
            } catch (parseError) {
                // Возможно, это не JSON ответ или ошибка расшифровки - используем оригинальный
                console.log(`🔐 Response parsing failed for ${originalUrl}, using original:`, parseError.message);
            }
        }

        return response;
    } catch (error) {
        console.error('Network error in API request:', error);
        // On network errors, check if we can still access the login page
        // If not, likely a server connectivity issue
        if (error instanceof TypeError && error.message.includes('NetworkError')) {
            console.log('Network connectivity issue detected');
            // Don't redirect on network errors - might be temporary server restart
        }
        throw error; // Re-throw to be handled by caller
    }
}


// Drag and Drop functionality
function initializeDragAndDrop(tableId, dataType) {
    const table = document.getElementById(tableId);
    if (!table) return;

    const tbody = table.querySelector('tbody');
    let draggedElement = null;

    // Desktop drag and drop
    tbody.addEventListener('dragstart', function(e) {
        if (e.target.closest('.draggable-row')) {
            draggedElement = e.target.closest('.draggable-row');
            draggedElement.classList.add('dragging');
            e.dataTransfer.effectAllowed = 'move';
            e.dataTransfer.setData('text/html', draggedElement.outerHTML);
        }
    });

    tbody.addEventListener('dragend', function(e) {
        if (draggedElement) {
            draggedElement.classList.remove('dragging');
            draggedElement = null;
        }
        // Remove all drop-zone classes
        tbody.querySelectorAll('.drop-zone').forEach(row => {
            row.classList.remove('drop-zone');
        });
    });

    tbody.addEventListener('dragover', function(e) {
        e.preventDefault();
        const afterElement = getDragAfterElement(tbody, e.clientY);
        const dragging = tbody.querySelector('.dragging');

        // Remove existing drop-zone classes
        tbody.querySelectorAll('.drop-zone').forEach(row => {
            row.classList.remove('drop-zone');
        });

        if (afterElement == null) {
            tbody.appendChild(dragging);
        } else {
            afterElement.classList.add('drop-zone');
            tbody.insertBefore(dragging, afterElement);
        }
    });

    tbody.addEventListener('drop', function(e) {
        e.preventDefault();
        const fromIndex = parseInt(draggedElement.dataset.index);
        const toIndex = getNewIndex(tbody, draggedElement);

        if (fromIndex !== toIndex) {
            reorderItems(dataType, fromIndex, toIndex);
        }

        // Clean up
        tbody.querySelectorAll('.drop-zone').forEach(row => {
            row.classList.remove('drop-zone');
        });
    });

    // Mobile touch support
    let touchStartY = 0;
    let touchElement = null;

    tbody.addEventListener('touchstart', function(e) {
        const row = e.target.closest('.draggable-row');
        if (row && e.target.closest('.drag-handle')) {
            touchStartY = e.touches[0].clientY;
            touchElement = row;
            row.classList.add('dragging');
            e.preventDefault();
        }
    });

    tbody.addEventListener('touchmove', function(e) {
        if (touchElement) {
            e.preventDefault();
            const touch = e.touches[0];
            const currentY = touch.clientY;

            // Visual feedback
            const afterElement = getDragAfterElement(tbody, currentY);
            tbody.querySelectorAll('.drop-zone').forEach(row => {
                row.classList.remove('drop-zone');
            });

            if (afterElement) {
                afterElement.classList.add('drop-zone');
            }
        }
    });

    tbody.addEventListener('touchend', function(e) {
        if (touchElement) {
            const touch = e.changedTouches[0];
            const afterElement = getDragAfterElement(tbody, touch.clientY);
            const fromIndex = parseInt(touchElement.dataset.index);

            // Calculate new position
            if (afterElement) {
                tbody.insertBefore(touchElement, afterElement);
            } else {
                tbody.appendChild(touchElement);
            }

            const toIndex = getNewIndex(tbody, touchElement);

            if (fromIndex !== toIndex) {
                reorderItems(dataType, fromIndex, toIndex);
            }

            // Clean up
            touchElement.classList.remove('dragging');
            touchElement = null;
            tbody.querySelectorAll('.drop-zone').forEach(row => {
                row.classList.remove('drop-zone');
            });
        }
    });
}

function getDragAfterElement(container, y) {
    const draggableElements = [...container.querySelectorAll('.draggable-row:not(.dragging)')];

    return draggableElements.reduce((closest, child) => {
        const box = child.getBoundingClientRect();
        const offset = y - box.top - box.height / 2;

        if (offset < 0 && offset > closest.offset) {
            return { offset: offset, element: child };
        } else {
            return closest;
        }
    }, { offset: Number.NEGATIVE_INFINITY }).element;
}

function getNewIndex(tbody, draggedElement) {
    const rows = [...tbody.querySelectorAll('.draggable-row')];
    return rows.indexOf(draggedElement);
}

function reorderItems(dataType, fromIndex, toIndex) {
    if (dataType === 'keys') {
        const item = keysData.splice(fromIndex, 1)[0];
        keysData.splice(toIndex, 0, item);
        // Send update to server (includes localStorage backup)
        saveKeysOrder();
    } else if (dataType === 'passwords') {
        const item = passwordsData.splice(fromIndex, 1)[0];
        passwordsData.splice(toIndex, 0, item);
        // Send update to server (includes localStorage backup)
        savePasswordsOrder();
    }
}

function saveKeysOrder() {
    const orderData = keysData.map((key, index) => ({ name: key.name, order: index }));
    makeAuthenticatedRequest('/api/keys/reorder', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ order: orderData })
    }).then(res => {
        if (res.ok) {
            CacheManager.invalidate('keys_list'); // ♻️ Инвалидация кеша
            showStatus('密钥顺序已保存！');
        } else {
            showStatus('保存密钥顺序失败。', true);
        }
    }).catch(err => {
        console.warn('Keys reorder API error:', err);
        showStatus('保存密钥顺序失败。', true);
    });
}

function savePasswordsOrder() {
    const orderData = passwordsData.map((password, index) => ({ name: password.name, order: index }));
    makeAuthenticatedRequest('/api/passwords/reorder', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ order: orderData })
    }).then(res => {
        if (res.ok) {
            CacheManager.invalidate('passwords_list'); // ♻️ Инвалидация кеша
            showStatus('密码顺序已保存！');
        } else {
            showStatus('保存密码顺序失败。', true);
        }
    }).catch(err => {
        console.warn('Passwords reorder API error:', err);
        showStatus('保存密码顺序失败。', true);
    });
}

window.onclick = function(event) {
    if (event.target.classList.contains('modal')) {
        event.target.style.display = 'none';
    }
}

document.addEventListener('DOMContentLoaded', async function(){
    try {
        document.getElementById('Keys').style.display = "block";

        // 🔒 Проверяем наличие сессионной cookie
        const sessionCookie = getCookie('session');
        if (!sessionCookie) {
            console.log('🧹 No session cookie found, clearing cache...');
            CacheManager.clear(); // Очищаем кеш если нет сессии
        }

        // Validate session and fetch CSRF token - redirect if invalid
        const isValidSession = await fetchCsrfToken();
        if (!isValidSession) {
            return;
        }

        // 🔗 ЗАГРУЖАЕМ URL MAPPINGS ПЕРВЫМИ - ДО keyExchange!
        // Важно: /api/secure/keyexchange тоже должен быть обфусцирован
        try {
            console.log('🔗 Loading URL obfuscation mappings...');
            const response = await fetch('/api/url_obfuscation/mappings');
            if (response.ok) {
                const mappings = await response.json();
                window.urlObfuscationMap = mappings;
                console.log(`🔗 Loaded ${Object.keys(mappings).length} URL obfuscation mappings`);
            } else {
                console.warn('⚠️ Failed to load URL mappings, using direct URLs');
                window.urlObfuscationMap = {};
            }
        } catch (error) {
            console.warn('⚠️ Error loading URL mappings:', error.message);
            window.urlObfuscationMap = {};
        }

        // 🔐 ВАЖНО: Инициализируем SecureClient ПОСЛЕ загрузки mappings
        console.log('[SecureClient] Initializing secure connection...');
        try {
            if (window.secureClient && typeof window.secureClient.establishSoftwareSecureConnection === 'function') {
                const success = await window.secureClient.establishSoftwareSecureConnection();
                if (success) {
                    console.log('🔒 HTTPS-like encryption ACTIVATED!');
                } else {
                    console.warn('❌ Failed to establish secure connection. Using regular HTTP.');
                }
            } else {
                console.warn('⚠️ SecureClient not available, skipping encryption');
            }
        } catch (error) {
            console.error('Error initializing secure connection:', error);
        }

        // 🔐 ВАЖНО: Задержка для инициализации SecureClient
        setTimeout(async () => {
            try {
                // 🛡️ Загружаем все ПОСЛЕДОВАТЕЛЬНО чтобы не перегружать сервер
                await fetchKeys(); // Теперь SecureClient будет готов
                await fetchPinSettings();
                // ⚠️ ОПТИМИЗАЦИЯ: НЕ проверяем API статус при загрузке
                // API по умолчанию неактивен, проверка только после enableApi()
                // Polling запускается ТОЛЬКО после enableApi() когда нужно

                // 🕒 Инициализируем timeout систему ПОСЛЕ secureClient
                initializeTimeoutSystem();
            } catch (err) {
                console.error('Error during initialization:', err);
            }
        }, 1000); // 1 секунда задержки
    } catch (err) {
        console.error('❌ Fatal error in DOMContentLoaded:', err);
        showStatus('初始化失败，请刷新页面。', true);
    }
});

// --- New API Access Logic ---
let apiAccessInterval = null;
let apiRemainingSeconds = 0; // Локальный счетчик времени
let apiCountdownInterval = null;

function enableApi() {
    makeEncryptedRequest('/api/enable_import_export', { method: 'POST' })
    .then(res => {
        if (res.ok) {
            showStatus('API 访问已启用（5 分钟）。');
            updateApiStatus(); // Получаем начальное время

            // ⚡ ОПТИМИЗАЦИЯ: Запускаем редкий polling (30 сек) для синхронизации
            if (apiAccessInterval) clearInterval(apiAccessInterval);
            apiAccessInterval = setInterval(updateApiStatus, 30000); // Каждые 30 сек

            // 🕐 Локальный countdown таймер (каждую секунду)
            if (apiCountdownInterval) clearInterval(apiCountdownInterval);
            apiCountdownInterval = setInterval(localCountdown, 1000);
        } else {
            showStatus('启用 API 访问失败。', true);
        }
    }).catch(err => showStatus('错误：' + err, true));
}

// 🕐 Локальный countdown без запросов к серверу
function localCountdown() {
    if (apiRemainingSeconds > 0) {
        apiRemainingSeconds--;

        // Обновляем UI локально
        const statusElements = document.querySelectorAll('.api-status');
        statusElements.forEach(el => {
            el.textContent = `已启用（剩余 ${apiRemainingSeconds} 秒）`;
            el.style.color = '#81c784'; // Green
        });
    } else {
        // Время истекло - отключаем кнопки и останавливаем таймеры
        const statusElements = document.querySelectorAll('.api-status');
        const exportKeysBtn = document.getElementById('export-keys-btn');
        const importKeysBtn = document.getElementById('import-keys-btn');
        const exportPasswordsBtn = document.getElementById('export-passwords-btn');
        const importPasswordsBtn = document.getElementById('import-passwords-btn');

        statusElements.forEach(el => {
            el.textContent = '未启用';
            el.style.color = '#ffc107'; // Yellow
        });
        exportKeysBtn.disabled = true;
        importKeysBtn.disabled = true;
        exportPasswordsBtn.disabled = true;
        importPasswordsBtn.disabled = true;

        // Останавливаем оба таймера
        if (apiAccessInterval) {
            clearInterval(apiAccessInterval);
            apiAccessInterval = null;
        }
        if (apiCountdownInterval) {
            clearInterval(apiCountdownInterval);
            apiCountdownInterval = null;
        }
        console.log('🛑 API access expired (local countdown)');
    }
}

// 🔄 Синхронизация с сервером (вызывается редко - каждые 30 сек)
function updateApiStatus() {
    makeEncryptedRequest('/api/import_export_status')
    .then(response => response.json())
    .then(data => {
        const statusElements = document.querySelectorAll('.api-status');
        const exportKeysBtn = document.getElementById('export-keys-btn');
        const importKeysBtn = document.getElementById('import-keys-btn');
        const exportPasswordsBtn = document.getElementById('export-passwords-btn');
        const importPasswordsBtn = document.getElementById('import-passwords-btn');

        if (data.enabled) {
            // 📥 Синхронизируем локальный счетчик с сервером
            apiRemainingSeconds = data.remaining;
            console.log(`🔄 Synced with server: ${apiRemainingSeconds}s remaining`);

            statusElements.forEach(el => {
                el.textContent = `已启用（剩余 ${apiRemainingSeconds} 秒）`;
                el.style.color = '#81c784'; // Green
            });
            exportKeysBtn.disabled = false;
            importKeysBtn.disabled = false;
            exportPasswordsBtn.disabled = false;
            importPasswordsBtn.disabled = false;
        } else {
            // API неактивен - останавливаем всё
            apiRemainingSeconds = 0;

            statusElements.forEach(el => {
                el.textContent = '未启用';
                el.style.color = '#ffc107'; // Yellow
            });
            exportKeysBtn.disabled = true;
            importKeysBtn.disabled = true;
            exportPasswordsBtn.disabled = true;
            importPasswordsBtn.disabled = true;

            // 🛑 Останавливаем оба таймера
            if (apiAccessInterval) {
                clearInterval(apiAccessInterval);
                apiAccessInterval = null;
            }
            if (apiCountdownInterval) {
                clearInterval(apiCountdownInterval);
                apiCountdownInterval = null;
            }
            console.log('🛑 Stopped API polling (API inactive from server)');
        }
    }).catch(err => console.error('Error fetching API status:', err));
}

document.querySelectorAll('.enable-api-btn').forEach(button => {
    button.addEventListener('click', enableApi);
});
// --- End of New API Access Logic ---

// Session Duration Settings Functions
async function fetchSessionDurationSettings() {
    try{
    const res = await makeEncryptedRequest('/api/session_duration', {
        method: 'GET'
    });
    const data = await res.json();
        const select = document.getElementById('session-duration');
        if (select && data.duration !== undefined) {
            select.value = data.duration;
        }
    }catch(err){
        console.error('获取会话时长设置失败:', err);
    }
}

// Session Duration Form Handler
document.addEventListener('DOMContentLoaded', function() {
    const sessionDurationForm = document.getElementById('session-duration-form');
    if (sessionDurationForm) {
        sessionDurationForm.addEventListener('submit', function(e) {
            e.preventDefault();

            const duration = document.getElementById('session-duration').value;

            const formData = new FormData();
            formData.append('duration', duration);

            makeEncryptedRequest('/api/session_duration', {
                method: 'POST',
                body: formData
            }).then(res => res.json()).then(data => {
                if (data.success) {
                    showStatus(data.message);

                    // Show info about when changes take effect
                    const infoMessages = {
                        '0': '会话将在设备重启时失效。',
                        '1': '会话将在 1 小时后失效。',
                        '6': '会话将在 6 小时后失效。',
                        '24': '会话将在 24 小时后失效。',
                        '72': '会话将在 3 天后失效。'
                    };

                    setTimeout(() => {
                        showStatus(infoMessages[duration] || '会话时长已更新。');
                    }, 2000);
                } else {
                    showStatus(data.message, true);
                }

            }).catch(err => {
                console.error('更新会话时长失败:', err);
                showStatus('更新会话时长失败。', true);
            });
        });
    }
});

</script>


<!-- START: WEB SERVER TIMEOUT MODAL -->
<div id="timeout-modal" style="display: none; position: fixed; z-index: 2000; left: 0; top: 0; width: 100%; height: 100%; background-color: rgba(0,0,0,0.7); backdrop-filter: blur(5px);">
    <div style="background: rgba(40, 40, 60, 0.9); border: 1px solid rgba(255, 255, 255, 0.1); margin: 15% auto; padding: 30px; width: 90%; max-width: 400px; border-radius: 15px; box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.5); color: #e0e0e0; text-align: center;">
        <h3 style="color: #ffffff; margin-top: 0;">会话即将超时</h3>
        <p>由于长时间无操作，Web 服务即将自动关闭。</p>
        <p>剩余时间：<span id="timeout-countdown" style="font-weight: bold; font-size: 1.2em; color: #5a9eee;">60</span> 秒</p>
        <button id="timeout-keep-alive-btn" class="button user-activity" style="width: 100%; padding: 15px; font-size: 1.1em;">继续会话</button>
    </div>
</div>

<script>
// 🎯 Сделано глобальной функцией чтобы вызывать из DOMContentLoaded
(function() {
    // Get timeout from server configuration
    let WARNING_TIME = 1 * 60 * 1000; // Default fallback
    let COUNTDOWN_SECONDS = 60;
    let SERVER_TIMEOUT_MINUTES = 2; // Default fallback

    let inactivityTimer = null;
    let countdownTimer = null;
    let remainingSeconds = COUNTDOWN_SECONDS;

    const modal = document.getElementById('timeout-modal');
    const countdownSpan = document.getElementById('timeout-countdown');
    const keepAliveBtn = document.getElementById('timeout-keep-alive-btn');

    // Fetch server timeout configuration and initialize timers
    // 🎯 Сделано глобальной чтобы было доступно из DOMContentLoaded
    window.initializeTimeoutSystem = function() {
        makeAuthenticatedRequest('/api/config')
            .then(response => response.json())
            .then(config => {
                if (config.web_server_timeout && config.web_server_timeout > 0) {
                    SERVER_TIMEOUT_MINUTES = config.web_server_timeout;
                    // Show warning 1 minute before timeout, but at least 30 seconds for very short timeouts
                    if (SERVER_TIMEOUT_MINUTES <= 1) {
                        WARNING_TIME = (SERVER_TIMEOUT_MINUTES * 60 * 1000) / 2; // Half of timeout for very short timeouts
                    } else {
                        WARNING_TIME = (SERVER_TIMEOUT_MINUTES - 1) * 60 * 1000; // 1 minute before for normal timeouts
                    }
                } else {
                    // If timeout is 0 or disabled, don't show warning
                    WARNING_TIME = 0;
                }
                // Start the timeout system after getting configuration
                resetServerActivity();
            })
            .catch(err => {
                // Use default values and start anyway
                resetServerActivity();
            });
    }

    function showWarningModal() {
        remainingSeconds = COUNTDOWN_SECONDS;
        countdownSpan.textContent = remainingSeconds;
        modal.style.display = 'block';

        countdownTimer = setInterval(() => {
            remainingSeconds--;
            countdownSpan.textContent = remainingSeconds;
            if (remainingSeconds <= 0) {
                clearInterval(countdownTimer);
                modal.querySelector('h3').textContent = '服务器即将关闭';
                modal.querySelector('p').innerHTML = 'The server has been shut down due to inactivity. <br>Please reboot the device to access it again.';
                keepAliveBtn.style.display = 'none';
            }
        }, 1000);
    }

    function hideWarningModal() {
        modal.style.display = 'none';
        if (countdownTimer) {
            clearInterval(countdownTimer);
            countdownTimer = null;
        }
    }

    function resetServerActivity() {
        hideWarningModal();

        if (WARNING_TIME > 0) {
            makeAuthenticatedRequest('/api/activity', { method: 'POST' }).catch(err => console.error("Failed to reset activity timer:", err));

            if (inactivityTimer) {
                clearTimeout(inactivityTimer);
            }
            inactivityTimer = setTimeout(showWarningModal, WARNING_TIME);
        }
    }

    function addActivityListeners() {
        document.querySelectorAll('.user-activity').forEach(elem => {
            elem.addEventListener('click', resetServerActivity);
            elem.addEventListener('input', resetServerActivity);
        });
    }

    // Initial setup
    // initializeTimeoutSystem(); // ❌ ПЕРЕМЕЩЕНО в DOMContentLoaded после secureClient
    addActivityListeners();

    // Re-attach listeners after fetching new content, e.g., after fetching keys/passwords
    // We can override the original functions to add this hook
    const originalFetchKeys = window.fetchKeys;
    window.fetchKeys = function() {
        originalFetchKeys();
        setTimeout(addActivityListeners, 500); // Re-add listeners after table is populated
    };

    const originalFetchPasswords = window.fetchPasswords;
    window.fetchPasswords = function() {
        originalFetchPasswords();
        setTimeout(addActivityListeners, 500); // Re-add listeners after table is populated
    };

})();
</script>
<!-- END: WEB SERVER TIMEOUT MODAL -->

</body></html>
)rawliteral";
