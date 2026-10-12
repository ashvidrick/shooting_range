#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include "Target.h"
#include "SoundManager.h"
#include "Score.h"
#include "SettingsManager.h"

class Game {
public:
    Game();
    void run();

private:
    enum GameState { MENU, PLAYING, SETTINGS, ABOUT, GAMEOVER };
    GameState currentState;
    bool paused;
    bool showTooltips;      // показывать подсказки по клавишам
    int tempVolume;         // временная громкость в меню настроек
    int tempDifficulty;     // временная сложность в меню настроек
    int menuSelection;      // 0,1,2 для пунктов меню (Старт, Настройки, О программе)
    int settingsSelection;  // 0,1,2 для настроек (Громкость, Сложность, Подсказки)

    // Окно и ресурсы
    sf::RenderWindow window;
    sf::Texture targetTexture;
    sf::Texture backgroundTexture;
    sf::Texture crosshairTexture;
    sf::Sprite backgroundSprite;
    sf::Sprite crosshairSprite;

    // Игровые объекты
    std::vector<Target> targets;
    SoundManager soundManager;
    Score score;

    // Настройки
    Config config;
    int spawnDelayFrames;
    int frameCounter;

    // Шрифты и тексты
    sf::Font font;
    std::optional<sf::Text> menuText;
    std::optional<sf::Text> settingsText;
    std::optional<sf::Text> aboutText;
    std::optional<sf::Text> difficultyText;
    std::optional<sf::Text> pauseText;
    std::optional<sf::Text> volumeIndicator;   // показ уровня громкости при изменении
    float volumeIndicatorTimer;                 // таймер скрытия индикатора громкости

    // Приватные методы
    void loadResources();
    int getMenuItemAt(int x, int y);
    int getSettingsItemAt(int x, int y);
    void handleEvents();
    void updateSpawn();
    void spawnTarget();
    void updateTargets();
    void handleShot(int mouseX, int mouseY);
    void render();
    void updateDifficultyDisplay();
    void changeDifficulty(int newDifficulty);
    void adjustVolume(int delta);               // изменение громкости (+-)
    void showVolumeIndicator();

    // Методы для состояний
    void handleMenuEvents(const sf::Event::KeyPressed& event);
    void handleSettingsEvents(const sf::Event::KeyPressed& event);
    void handlePlayingEvents(const sf::Event::KeyPressed& event);
    void renderMenu();
    void renderSettings();
    void renderAbout();
    void renderGame();
    void renderPauseOverlay();
    void renderTooltips();                      // если showTooltips true


    bool timedMode;          // true – режим с таймером
    int targetScore;         // целевое количество очков
    float timeRemaining;     // оставшееся время (в секундах)
    sf::Clock gameClock;     // для отсчёта времени
    bool gameFinished;       // флаг завершения игры (победа/поражение)
    std::optional<sf::Text> timerText;   // отображение таймера
    std::optional<sf::Text> targetText;  // отображение цели

    float gameOverTimer;
    sf::Clock gameOverClock;

    void checkGameEnd();     // проверка условий победы/поражения
    void resetGame();        // сброс состояния перед запуском
    void returnToMenu();     // выход в главное меню с сохранением
    void startGame();          // запуск новой игры
    void renderGameOver();     // отрисовка экрана завершения

    int timeLimit;
};