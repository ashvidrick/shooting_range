#pragma once

#include <string>

struct Config {
    int volume;       // 0..100
    int difficulty;   // 1,2,3
    int gameMode;     // 0 - бесконечный, 1 - с таймером
    int targetScore;  // целевое количество очков
    int timeLimit;    // время в секундах (для режима с таймером)
};

class SettingsManager {
public:
    static Config loadConfig(const std::string& filename = "settings.ini");
    static void saveConfig(const Config& cfg, const std::string& filename = "settings.ini");
};
