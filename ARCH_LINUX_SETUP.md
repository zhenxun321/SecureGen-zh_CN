# 🐧 Arch Linux Setup Instructions

## Шаг 1: Перейти в директорию проекта

```bash
cd ~/Downloads/production\ copy/T-Disp\[TOTP\]/production2/dev2/T-Disp-TOTP
```

## Шаг 2: Установить GitHub CLI

```bash
sudo pacman -S github-cli
```

## Шаг 3: Авторизоваться в GitHub

```bash
gh auth login
```

Выберите:
- GitHub.com
- HTTPS
- Login with a web browser (или используйте токен)

## Шаг 4: Запустить скрипт настройки

```bash
bash .github/setup-repo.sh
```

---

## ✅ Готово!

После выполнения скрипта проверьте репозиторий:
https://github.com/Unix-like-SoN/SecureGen

---

## 🔍 Проверка установки

Проверить что GitHub CLI установлен:
```bash
gh --version
```

Проверить авторизацию:
```bash
gh auth status
```

Посмотреть информацию о репозитории:
```bash
gh repo view Unix-like-SoN/SecureGen
```

---

## 📝 Примечания

- Команды для Debian/Ubuntu (apt, dpkg) НЕ работают на Arch Linux
- На Arch используется пакетный менеджер `pacman`
- Все инструкции для Arch уже есть в `.github/QUICK_SETUP.md`
