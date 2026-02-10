#ifndef KEY_MANAGER_H
#define KEY_MANAGER_H

#include <vector>
#include <Arduino.h>

// Структура для хранения ключа
struct TOTPKey {
    String name;
    String secret;
    int order = 0;  // Порядок сортировки
};

class KeyManager {
public:
    KeyManager();
    bool begin(); // Загружает ключи в память при старте
    
    // Функции для управления ключами
    bool addKey(const String& name, const String& secret);
    bool removeKey(int index);
    bool updateKey(int index, const String& name, const String& secret); // <-- ADDED
    bool reorderKeys(const std::vector<std::pair<String, int>>& newOrder); // Изменение порядка
    std::vector<TOTPKey> getAllKeys();
    bool replaceAllKeys(const String& jsonContent); // Новая функция

private:
    bool loadKeys();
    bool saveKeys();

    std::vector<TOTPKey> keys; // Ключи хранятся в памяти в расшифрованном виде
};

#endif // KEY_MANAGER_H