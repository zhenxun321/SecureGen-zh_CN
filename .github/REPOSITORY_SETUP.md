# GitHub Repository Setup Guide

This guide helps you configure the GitHub repository metadata, topics, and settings.

---

## 📋 Repository Information

### Basic Info

**Name:** `SecureGen`

**Description:**
```
🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display
```

**Website:** (Optional)
```
https://github.com/Unix-like-SoN/SecureGen
```

---

## 🏷️ Topics (Tags)

Add these topics to improve discoverability:

### Primary Topics
```
esp32
totp
password-manager
security
authentication
2fa
two-factor-authentication
```

### Technology Topics
```
arduino
platformio
embedded
iot
hardware
lilygo
t-display
```

### Feature Topics
```
encryption
aes-256
bluetooth-le
ble
offline
air-gapped
open-source
```

### Use Case Topics
```
authenticator
otp
password-vault
security-device
hardware-security
```

---

## ⚙️ Repository Settings

### General Settings

**Features to Enable:**
- ✅ Issues
- ✅ Discussions
- ✅ Projects (optional)
- ✅ Wiki (optional)
- ✅ Sponsorships (already configured via FUNDING.yml)

**Features to Disable:**
- ❌ Wikis (if using docs/ folder instead)

### Pull Requests

**Settings:**
- ✅ Allow squash merging
- ✅ Allow merge commits
- ✅ Allow rebase merging
- ✅ Automatically delete head branches

### Security

**Settings:**
- ✅ Private vulnerability reporting (enable)
- ✅ Dependency graph
- ✅ Dependabot alerts
- ✅ Dependabot security updates

---

## 🔧 Setup Methods

### Method 1: GitHub Web Interface (Recommended)

1. **Go to Repository Settings**
   - Navigate to your repository
   - Click "Settings" tab

2. **Update Description**
   - In "About" section (right sidebar on main page)
   - Click gear icon ⚙️
   - Paste description
   - Add website URL
   - Add topics (see list above)
   - Check "Releases" and "Packages"

3. **Configure Features**
   - Settings → General → Features
   - Enable/disable as listed above

4. **Security Settings**
   - Settings → Security → Code security and analysis
   - Enable recommended features

### Method 2: GitHub CLI (gh)

If you have GitHub CLI installed:

```bash
# Install GitHub CLI first if needed
# https://cli.github.com/

# Login to GitHub
gh auth login

# Set repository description
gh repo edit Unix-like-SoN/SecureGen \
  --description "🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display"

# Add topics (run multiple times for each topic)
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

# Enable features
gh repo edit Unix-like-SoN/SecureGen \
  --enable-issues \
  --enable-discussions

# Set homepage
gh repo edit Unix-like-SoN/SecureGen \
  --homepage "https://github.com/Unix-like-SoN/SecureGen"
```

### Method 3: GitHub API (curl)

```bash
# Set your GitHub token
GITHUB_TOKEN="your_personal_access_token"
REPO="Unix-like-SoN/SecureGen"

# Update repository
curl -X PATCH \
  -H "Authorization: token $GITHUB_TOKEN" \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/$REPO \
  -d '{
    "description": "🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display",
    "homepage": "https://github.com/Unix-like-SoN/SecureGen",
    "topics": [
      "esp32", "totp", "password-manager", "security", "authentication",
      "2fa", "two-factor-authentication", "arduino", "platformio", "embedded",
      "iot", "hardware", "lilygo", "t-display", "encryption", "aes-256",
      "bluetooth-le", "ble", "offline", "air-gapped", "open-source",
      "authenticator", "otp", "password-vault", "security-device", "hardware-security"
    ],
    "has_issues": true,
    "has_discussions": true,
    "has_projects": false,
    "has_wiki": false
  }'
```

---

## 📱 Social Preview Image

Create a social preview image (1280x640px) showing:
- Device photo
- Project name
- Key features
- Tech stack icons

Upload via: Settings → General → Social preview → Upload an image

---

## 🎯 About Section Configuration

**In the "About" section (right sidebar), configure:**

1. **Description:** (paste from above)
2. **Website:** Repository URL or custom domain
3. **Topics:** All topics from list above
4. **Releases:** ✅ Check
5. **Packages:** ✅ Check (if using)
6. **Deployments:** ❌ Uncheck (not applicable)

---

## 📊 Insights Configuration

**Enable in Settings → Insights:**
- ✅ Traffic
- ✅ Visitors
- ✅ Clones
- ✅ Popular content

---

## 🔔 Notifications

**Recommended settings:**
- Watch: Custom → Releases only
- Discussions: All activity
- Issues: Participating and @mentions

---

## 🎨 Repository Labels

**Suggested custom labels:**

```
Type: Security 🔒 - #d73a4a
Type: Feature ✨ - #0075ca
Type: Bug 🐛 - #d73a4a
Type: Documentation 📚 - #0075ca
Priority: High 🔥 - #d93f0b
Priority: Medium ⚡ - #fbca04
Priority: Low 🌱 - #0e8a16
Status: In Progress 🚧 - #fbca04
Status: Needs Review 👀 - #0075ca
Hardware: T-Display 📱 - #5319e7
Component: Crypto 🔐 - #d73a4a
Component: Web 🌐 - #0075ca
Component: BLE 📡 - #5319e7
Good First Issue 👋 - #7057ff
Help Wanted 🙋 - #008672
```

---

## 📋 Issue Templates

Already configured via `.github/ISSUE_TEMPLATE/` (if exists).

**Recommended templates:**
1. Bug Report
2. Feature Request
3. Security Vulnerability
4. Documentation Improvement

---

## 🎯 Discussion Categories

**Recommended categories:**
1. 📢 Announcements
2. 💡 Ideas & Feature Requests
3. 🙏 Q&A
4. 🎉 Show and Tell
5. 🔒 Security
6. 🐛 Troubleshooting

---

## ✅ Verification Checklist

After setup, verify:

- [ ] Description is visible on main page
- [ ] Topics are displayed
- [ ] License badge shows "MIT"
- [ ] Sponsor button appears
- [ ] Issues are enabled
- [ ] Discussions are enabled
- [ ] Security policy is visible
- [ ] README renders correctly
- [ ] Images load properly
- [ ] Links work

---

## 🚀 Post-Setup

1. **Create First Release**
   ```bash
   git tag -a v1.0.0 -m "Initial release"
   git push origin v1.0.0
   ```

2. **Write Release Notes**
   - Go to Releases → Draft a new release
   - Use tag v1.0.0
   - Add changelog
   - Attach compiled binaries (optional)

3. **Enable GitHub Pages** (optional)
   - Settings → Pages
   - Source: docs/ folder or gh-pages branch
   - Custom domain (optional)

4. **Set up Branch Protection** (optional)
   - Settings → Branches
   - Add rule for `main`
   - Require pull request reviews
   - Require status checks

---

## 📞 Need Help?

- GitHub Docs: https://docs.github.com/
- GitHub CLI: https://cli.github.com/manual/
- GitHub API: https://docs.github.com/en/rest

---

**Last Updated:** February 2025
