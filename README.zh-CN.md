# SecureGen（中文说明）

> ESP32 T-Display 多功能安全设备：集成 TOTP 动态口令、离线密码管理与 BLE 键盘传输。

## 项目简介

SecureGen 是一个开源硬件安全项目，目标是在离线场景下提供更安全的双因素认证与密码管理能力。

核心能力：
- TOTP 身份验证器（兼容标准 2FA 服务）
- 本地离线密码库
- 通过 BLE 键盘安全传输密码
- Web 管理后台（增删改查、导入导出、主题与设备设置）
- 多层通信防护（加密、混淆、会话保护等）

## 快速开始

1. 安装 [PlatformIO IDE](https://platformio.org/platformio-ide)（推荐 VS Code 插件）。
2. 克隆仓库并打开项目目录。
3. 连接 ESP32 T-Display 开发板。
4. 编译并烧录：

```bash
pio run -t upload
```

5. 打开串口监视器查看启动日志：

```bash
pio device monitor
```

## 常用文档

- 总文档索引：`docs/README.md`
- 安全架构：`docs/security/SECURITY_OVERVIEW.md`
- HTTPS 集成：`docs/development/https-integration-guide.md`
- 运行模式：`docs/features/operating-modes.md`

## 汉化说明

当前仓库已开始中文化，优先覆盖：
- Web 登录页中文文案
- 主管理页（部分）中文文案
- 中文 README 入口

欢迎继续提交 PR 完善其它页面与开发文档的中文翻译。
