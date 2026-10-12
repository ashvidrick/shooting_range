#include "SettingsManager.h"
#include <windows.h>
#include <cstdlib>

Config SettingsManager::loadConfig(const std::string& filename) {
    Config cfg;
    char buffer[10];

    // Чтение громкости
    GetPrivateProfileStringA("Audio", "Volume", "75", buffer, sizeof(buffer), filename.c_str());
    cfg.volume = std::atoi(buffer);
    if (cfg.volume < 0) cfg.volume = 0;
    if (cfg.volume > 100) cfg.volume = 100;

    // Чтение сложности
    GetPrivateProfileStringA("Game", "Difficulty", "2", buffer, sizeof(buffer), filename.c_str());
    cfg.difficulty = std::atoi(buffer);
    if (cfg.difficulty < 1) cfg.difficulty = 1;
    if (cfg.difficulty > 3) cfg.difficulty = 3;

    // Чтение режима
    GetPrivateProfileStringA("Game", "Mode", "0", buffer, sizeof(buffer), filename.c_str());
    cfg.gameMode = std::atoi(buffer);
    if (cfg.gameMode < 0) cfg.gameMode = 0;
    if (cfg.gameMode > 1) cfg.gameMode = 1;

    // Чтение целевого счёта
    GetPrivateProfileStringA("Game", "TargetScore", "50", buffer, sizeof(buffer), filename.c_str());
    cfg.targetScore = std::atoi(buffer);
    if (cfg.targetScore < 1) cfg.targetScore = 1;

    // Чтение времени
    GetPrivateProfileStringA("Game", "TimeLimit", "60", buffer, sizeof(buffer), filename.c_str());
    cfg.timeLimit = std::atoi(buffer);
    if (cfg.timeLimit < 1) cfg.timeLimit = 1;

    return cfg;
}

void SettingsManager::saveConfig(const Config& cfg, const std::string& filename) {
    char volStr[4], diffStr[4];
    sprintf_s(volStr, sizeof(volStr), "%d", cfg.volume);
    sprintf_s(diffStr, sizeof(diffStr), "%d", cfg.difficulty);

    WritePrivateProfileStringA("Audio", "Volume", volStr, filename.c_str());
    WritePrivateProfileStringA("Game", "Difficulty", diffStr, filename.c_str());

    char modeStr[4], targetStr[8];
    sprintf_s(modeStr, sizeof(modeStr), "%d", cfg.gameMode);
    sprintf_s(targetStr, sizeof(targetStr), "%d", cfg.targetScore);
    WritePrivateProfileStringA("Game", "Mode", modeStr, filename.c_str());
    WritePrivateProfileStringA("Game", "TargetScore", targetStr, filename.c_str());
    char timeStr[8];
    sprintf_s(timeStr, sizeof(timeStr), "%d", cfg.timeLimit);
    WritePrivateProfileStringA("Game", "TimeLimit", timeStr, filename.c_str());
}
