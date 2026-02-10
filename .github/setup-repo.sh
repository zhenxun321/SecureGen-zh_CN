#!/bin/bash
# GitHub Repository Setup Script
# This script configures repository metadata using GitHub CLI

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Repository details
REPO_OWNER="Unix-like-SoN"
REPO_NAME="SecureGen"
REPO_FULL="${REPO_OWNER}/${REPO_NAME}"

echo -e "${GREEN}=== GitHub Repository Setup ===${NC}"
echo ""

# Check if gh CLI is installed
if ! command -v gh &> /dev/null; then
    echo -e "${RED}Error: GitHub CLI (gh) is not installed${NC}"
    echo "Install from: https://cli.github.com/"
    exit 1
fi

# Check if authenticated
if ! gh auth status &> /dev/null; then
    echo -e "${YELLOW}Not authenticated. Running gh auth login...${NC}"
    gh auth login
fi

echo -e "${GREEN}✓ GitHub CLI is ready${NC}"
echo ""

# Set repository description
echo -e "${YELLOW}Setting repository description...${NC}"
gh repo edit ${REPO_FULL} \
  --description "🔐 Hardware TOTP Authenticator & Password Manager | 7-layer security | AES-256 | BLE Keyboard | Offline | ESP32 T-Display" \
  && echo -e "${GREEN}✓ Description updated${NC}" \
  || echo -e "${RED}✗ Failed to update description${NC}"

# Set homepage
echo -e "${YELLOW}Setting homepage URL...${NC}"
gh repo edit ${REPO_FULL} \
  --homepage "https://github.com/${REPO_FULL}" \
  && echo -e "${GREEN}✓ Homepage updated${NC}" \
  || echo -e "${RED}✗ Failed to update homepage${NC}"

# Add topics
echo -e "${YELLOW}Adding topics...${NC}"
TOPICS=(
  "esp32"
  "totp"
  "password-manager"
  "security"
  "authentication"
  "2fa"
  "two-factor-authentication"
  "arduino"
  "platformio"
  "embedded"
  "iot"
  "hardware"
  "lilygo"
  "t-display"
  "encryption"
  "aes-256"
  "bluetooth-le"
  "ble"
  "offline"
  "air-gapped"
  "open-source"
  "authenticator"
  "otp"
  "password-vault"
  "security-device"
  "hardware-security"
)

for topic in "${TOPICS[@]}"; do
  gh repo edit ${REPO_FULL} --add-topic "${topic}" 2>/dev/null \
    && echo -e "${GREEN}  ✓ Added topic: ${topic}${NC}" \
    || echo -e "${YELLOW}  ⚠ Topic already exists or failed: ${topic}${NC}"
done

# Enable features
echo -e "${YELLOW}Enabling repository features...${NC}"
gh repo edit ${REPO_FULL} \
  --enable-issues \
  --enable-discussions \
  && echo -e "${GREEN}✓ Features enabled${NC}" \
  || echo -e "${RED}✗ Failed to enable features${NC}"

echo ""
echo -e "${GREEN}=== Setup Complete! ===${NC}"
echo ""
echo "Next steps:"
echo "1. Go to repository settings to verify changes"
echo "2. Add social preview image (1280x640px)"
echo "3. Configure discussion categories"
echo "4. Create first release"
echo ""
echo "Repository URL: https://github.com/${REPO_FULL}"
