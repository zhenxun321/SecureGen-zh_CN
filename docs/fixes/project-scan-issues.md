# 项目扫描：可能存在的问题（初步）

> 扫描范围：静态阅读 `src/`、`include/` 关键安全路径，未在真实硬件上运行。

## 1) 前后端加密协议不一致（高风险，可能导致握手失败）

- 前端 `auto_secure_initializer.js` 使用 **AES-GCM + 12 字节 IV + Base64 拼接格式**（salt+iv+ciphertext）。
- 后端 `CryptoManager::encryptWithPassword/decryptWithPassword` 使用 **AES-CBC + 16 字节 IV + JSON 格式**（`{salt,iv,ciphertext}`）。

这意味着即使密码相同，双方也无法互相解密，`/secure/protected-handshake` 可能出现间歇性或稳定失败。

## 2) PBKDF2 迭代次数不一致（中高风险）

- 前端固定 `iterations: 10000`。
- 后端 `PBKDF2_ITERATIONS_EXPORT` 配置为 `15000`。

这会直接导致派生密钥不同，进一步放大握手失败概率。

## 3) Method Tunneling 的 XOR 十六进制编码存在解码歧义（中风险）

`xorEncryptMethod` 使用 `String(encryptedChar, HEX)` 追加，未保证每字节固定两位 hex。若出现负值/高位字符，可能输出非两位内容；而 `xorDecryptMethod` 又按“两位一组”读取，存在解码错位风险。

## 4) 存在可预测默认 AP 密码回退（中风险）

在配置文件缺失、解析失败、解密失败时，都会回落到固定字符串 `12345678`。该行为虽然提升可恢复性，但降低出厂/异常场景下的安全基线。

## 5) 前端调试日志默认开启，可能泄露敏感上下文（中风险）

`auto_secure_initializer.js` 中 `DEBUG: true`，并输出指纹信息、握手过程与客户端标识片段。生产环境中建议默认关闭，并按构建环境开启。

---

## 建议优先级

1. **先统一握手协议**：算法（GCM/CBC）、IV 长度、数据封装格式、KDF 参数。
2. **再修复 method tunneling 编码**：统一为固定两位 hex（例如 `padStart(2,'0')`）或改为 Base64。
3. **最后收紧默认安全策略**：移除固定 AP 默认口令，改为首次启动随机口令 + 屏幕显示；关闭前端默认 DEBUG。
