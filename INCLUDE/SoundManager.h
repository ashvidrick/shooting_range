#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <optional>

class SoundManager {
public:
    SoundManager();
    void loadSounds(const std::string& shootFile, const std::string& hitFile);
    void setVolume(int volume);
    int getVolume() const { return currentVolume; }
    void playShoot();
    void playHit();

private:
    sf::SoundBuffer shootBuffer;
    sf::SoundBuffer hitBuffer;
    std::optional<sf::Sound> shootSound;
    std::optional<sf::Sound> hitSound;
    int currentVolume = 75;
};