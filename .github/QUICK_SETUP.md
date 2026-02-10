# Quick Setup Commands

Copy and paste these commands to quickly set up your GitHub repository.

---

## 🚀 One-Line Setup (Recommended)

If you have GitHub CLI installed:

```bash
bash .github/setup-repo.sh
```

---

## 📋 Manual Setup Commands

### 1. Install GitHub CLI (if not installed)

**macOS:**
```bash
brew install gh
```

**Linux (Debian/Ubuntu):**
```bash
curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
sudo apt update
sudo apt install gh
```

**Linux (Arch/Manjaro):**
```bash
sudo pacman -S github-cli
```

**Linux (Fedora/RHEL):**
```bash
sudo dnf install gh
```

**Windows:**
```powershell
winget install --id GitHub.cli
```

### 2. Login to GitHub

```bash
gh auth login
```

### 3. Set Repository Info

```bash
# Set repository description
gh repo edit Unix-like-SoN/SecureGen \
  --description "🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display"

# Set homepage
gh repo edit Unix-like-SoN/SecureGen \
  --homepage "https://github.com/Unix-like-SoN/SecureGen"

# Enable features
gh repo edit Unix-like-SoN/SecureGen \
  --enable-issues \
  --enable-discussions
```

### 4. Add Topics (All at Once)

```bash
gh repo edit Unix-like-SoN/SecureGen \
  --add-topic esp32 \
  --add-topic totp \
  --add-topic password-manager \
  --add-topic security \
  --add-topic authentication \
  --add-topic 2fa \
  --add-topic two-factor-authentication \
  --add-topic arduino \
  --add-topic platformio \
  --add-topic embedded \
  --add-topic iot \
  --add-topic hardware \
  --add-topic lilygo \
  --add-topic t-display \
  --add-topic encryption \
  --add-topic aes-256 \
  --add-topic bluetooth-le \
  --add-topic ble \
  --add-topic offline \
  --add-topic air-gapped \
  --add-topic open-source \
  --add-topic authenticator \
  --add-topic otp \
  --add-topic password-vault \
  --add-topic security-device \
  --add-topic hardware-security
```

---

## 🌐 Web Interface Setup (No CLI needed)

### Step 1: Update About Section

1. Go to: https://github.com/Unix-like-SoN/SecureGen
2. Click ⚙️ (gear icon) in "About" section (right sidebar)
3. Paste this description:
   ```
   🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display
   ```
4. Add website: `https://github.com/Unix-like-SoN/SecureGen`
5. Click "Topics" and add these (comma-separated):
   ```
   esp32, totp, password-manager, security, authentication, 2fa, two-factor-authentication, arduino, platformio, embedded, iot, hardware, lilygo, t-display, encryption, aes-256, bluetooth-le, ble, offline, air-gapped, open-source, authenticator, otp, password-vault, security-device, hardware-security
   ```
6. Check ✅ "Releases"
7. Click "Save changes"

### Step 2: Enable Features

1. Go to: https://github.com/Unix-like-SoN/SecureGen/settings
2. Under "Features":
   - ✅ Issues
   - ✅ Discussions
   - ❌ Projects (optional)
   - ❌ Wiki (using docs/ instead)
3. Scroll down and click "Save"

### Step 3: Configure Security

1. Go to: Settings → Security → Code security and analysis
2. Enable:
   - ✅ Dependency graph
   - ✅ Dependabot alerts
   - ✅ Dependabot security updates
   - ✅ Private vulnerability reporting

---

## 📊 Verify Setup

Check that everything is configured:

```bash
# View repository info
gh repo view Unix-like-SoN/SecureGen

# View topics
gh api repos/Unix-like-SoN/SecureGen --jq '.topics'
```

Or visit: https://github.com/Unix-like-SoN/SecureGen

---

## 🎨 Optional: Add Social Preview

1. Create image (1280x640px) with:
   - Device photo
   - Project name
   - Key features
2. Go to: Settings → General → Social preview
3. Upload image

---

## 🏷️ Create First Release

```bash
# Tag the release
git tag -a v1.0.0 -m "Initial production release"
git push origin v1.0.0

# Create release on GitHub
gh release create v1.0.0 \
  --title "v1.0.0 - Initial Release" \
  --notes "First production-ready release with full security features"
```

---

## ✅ Quick Checklist

After running setup:

- [ ] Description visible on main page
- [ ] Topics displayed under description
- [ ] License shows "MIT"
- [ ] Sponsor button appears
- [ ] Issues enabled
- [ ] Discussions enabled
- [ ] Security policy visible
- [ ] README renders correctly
- [ ] All images load

---

## 🆘 Troubleshooting

**"gh: command not found"**
- Install GitHub CLI: https://cli.github.com/

**"authentication required"**
- Run: `gh auth login`

**"permission denied"**
- Make script executable: `chmod +x .github/setup-repo.sh`

**Topics not showing**
- Wait a few minutes for GitHub to update
- Refresh the page

---

**Need help?** Open an issue or discussion!
