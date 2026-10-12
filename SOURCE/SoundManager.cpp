#include "SoundManager.h"
#include <stdexcept>

SoundManager::SoundManager() {
    // Конструктор пуст — звуки ещё не созданы
}

void SoundManager::loadSounds(const std::string& shootFile, const std::string& hitFile) {
    // Загружаем буферы (SFML 3: openFromFile)
    if (!shootBuffer.loadFromFile(shootFile))
        throw std::runtime_error("Failed to load " + shootFile);
    if (!hitBuffer.loadFromFile(hitFile))
        throw std::runtime_error("Failed to load " + hitFile);

    // Создаём звуки, передавая буферы в конструктор
    shootSound.emplace(shootBuffer);
    hitSound.emplace(hitBuffer);

    setVolume(currentVolume);
}

void SoundManager::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    currentVolume = volume;
    if (shootSound.has_value())
        shootSound->setVolume(static_cast<float>(volume));
    if (hitSound.has_value())
        hitSound->setVolume(static_cast<float>(volume));
}

void SoundManager::playShoot() {
    if (shootSound.has_value())
        shootSound->play();
}

void SoundManager::playHit() {
    if (hitSound.has_value())
        hitSound->play();
}