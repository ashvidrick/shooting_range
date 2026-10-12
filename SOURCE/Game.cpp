#include "tools.h"
#include "Game.h"

#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <windows.h> // для MessageBox
#include <SFML/Window/Window.hpp>
#include <iostream>

#include <algorithm>
#include <iostream>
#include <filesystem>

Game::Game()
    : window(sf::VideoMode({ 800, 600 }), L"Тир", sf::Style::Close)
    , currentState(MENU)
    , paused(false)
    , showTooltips(true)
    , tempVolume(75)
    , tempDifficulty(2)
    , menuSelection(0)
    , settingsSelection(0)
    , frameCounter(0)
    , volumeIndicatorTimer(0.f)
    , backgroundSprite(backgroundTexture)
    , crosshairSprite(crosshairTexture)
    , timedMode(false)
    , targetScore(50)
    , timeRemaining(60.f)
    , gameFinished(false)
    , gameOverTimer(0.f)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::string basePath = getExecutablePath();
    config = SettingsManager::loadConfig(basePath + "settings.ini");
    tempVolume = config.volume;
    tempDifficulty = config.difficulty;
    timedMode = (config.gameMode == 1);
    targetScore = config.targetScore;
    timedMode = (config.gameMode == 1);
    timeLimit = config.timeLimit;                    
    timeRemaining = static_cast<float>(timeLimit);   

    loadResources();

    difficultyText.emplace(font, "", 24);
    difficultyText->setFillColor(sf::Color::White);
    difficultyText->setPosition({ 10.f, 50.f });

    soundManager.setVolume(config.volume);
    changeDifficulty(config.difficulty);

    window.setMouseCursorVisible(true);

    // Загрузка шрифта (если не загрузится, программа упадёт)
    if (!font.openFromFile(basePath + "res/arial.ttf")) {
        throw std::runtime_error("Unable to load " + basePath +"res/arial.ttf");
    }

    // Создание текстов для меню, настроек и пр.
    menuText.emplace(font, L"", 28);
    menuText->setFillColor(sf::Color::White);
    settingsText.emplace(font, L"", 24);
    settingsText->setFillColor(sf::Color::White);
    aboutText.emplace(font, L"Тир\nВерсия 1.0\n\n\nУправление:\nЛКМ - Выстрел\nF1/F2/F3 - Изменить сложность\nВверх\\Вниз - Громкость\nP\\Пробел - пауза", 20);
    aboutText->setFillColor(sf::Color::White);
    aboutText->setPosition({ 50.f, 100.f });

    
    updateDifficultyDisplay();

    pauseText.emplace(font, L"Пауза", 50);
    pauseText->setFillColor(sf::Color::Red);
    pauseText->setPosition({ 300.f, 250.f });

    volumeIndicator.emplace(font, L"", 24);
    volumeIndicator->setFillColor(sf::Color::Yellow);
    volumeIndicator->setPosition({ 10.f, 100.f });
}

void Game::loadResources() {
    std::string basePath = getExecutablePath();
    // Загрузка текстур
         //(basePath + "res\\target.jpg"))
    if (!targetTexture.loadFromFile(basePath + "res/target.png"))
        throw std::runtime_error("Unable to load " + basePath + "res/target.png");
    if (!backgroundTexture.loadFromFile(basePath + "res/background.png"))
        throw std::runtime_error("Unable to load " + basePath + "res/background.png");
    if (!crosshairTexture.loadFromFile(basePath + "res/crosshair.png"))
        throw std::runtime_error("Unable to load " + basePath + "res/crosshair.png");

    backgroundSprite.setTexture(backgroundTexture, true);
    crosshairSprite.setTexture(crosshairTexture, true);

    soundManager.loadSounds(basePath + "res/shoot.wav", basePath + "res/hit.wav");
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        handleEvents();

        if (currentState == GAMEOVER) {
            gameOverTimer -= gameOverClock.restart().asSeconds();
            if (gameOverTimer <= 0.f) {
                returnToMenu();
            }
        }

        if (currentState == PLAYING && !paused && !gameFinished) {
            updateSpawn();
            updateTargets();
            // Обновление таймера
            if (timedMode) {
                timeRemaining -= clock.restart().asSeconds();
                if (timeRemaining <= 0.f) {
                    timeRemaining = 0.f;
                    // Поражение
                    gameFinished = true;
                    currentState = GAMEOVER;
                    gameOverTimer = 5.f;
                    gameOverClock.restart();                    
                }
                // Проверка победы уже делается в handleShot,
                // но на всякий случай можно продублировать:
                if (score.getScore() >= targetScore && !gameFinished) {
                    gameFinished = true;
                    currentState = GAMEOVER;
                    gameOverTimer = 5.f;
                    gameOverClock.restart();
                }
            }
            else {
                clock.restart();
            }
        }

        render();

        // Таймер для индикатора громкости
        if (volumeIndicatorTimer > 0.f) {
            volumeIndicatorTimer -= clock.restart().asSeconds();
            if (volumeIndicatorTimer < 0.f) volumeIndicatorTimer = 0.f;
        }
        else {
            if (volumeIndicator.has_value() && volumeIndicator->getString() != L"")
                volumeIndicator->setString(L"");
        }

        sf::sleep(sf::milliseconds(5));
    }
}


// Определение пункта меню по координатам мыши
int Game::getMenuItemAt(int x, int y) {
    // Позиции пунктов меню (примерно)
    sf::FloatRect bounds;
    std::vector<std::wstring> items = { L"Старт", L"Настройки", L"О программе" };
    for (int i = 0; i < 3; ++i) {
        // Создаём временный текст для вычисления границ
        sf::Text tmp(font, items[i], 28);
        tmp.setPosition({ 350.f, 200.f + static_cast<float>(i) * 40.f });
        bounds = tmp.getGlobalBounds();
        if (bounds.contains({ static_cast<float>(x), static_cast<float>(y)})) return i;
    }
    return -1;
}

int Game::getSettingsItemAt(int x, int y) {
    // Настройки расположены колонкой: Volume, Difficulty, Show tooltips
    for (int i = 0; i < 3; ++i) {
        sf::Text tmp(font, "", 24);
        tmp.setPosition({ 150.f, 150.f + i * 40.f });
        // Достаточно проверить по Y в пределах высоты строки
        if (y >= 150 + i * 40 && y <= 150 + i * 40 + 30) return i;
    }
    return -1;
}

void Game::handleEvents() {
    while (const auto& event = window.pollEvent()) {
        // 1. Закрытие окна
        if (event.value().is<sf::Event::Closed>()) {
            // Сохраняем настройки в зависимости от состояния
            if (currentState == PLAYING || currentState == GAMEOVER) {
                config.volume = soundManager.getVolume();
                config.difficulty = tempDifficulty;
            }
            else {
                config.volume = tempVolume;
                config.difficulty = tempDifficulty;
            }
            std::string basePath = getExecutablePath();
            SettingsManager::saveConfig(config, basePath + "settings.ini");
            window.close();
        }

        // 2. Обработка нажатий клавиш
        else if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
            // --- Глобальные клавиши (работают везде) ---
            // Регулировка громкости только в игре (по желанию можно сделать везде)
            if (keyEvent->code == sf::Keyboard::Key::Up && currentState == PLAYING) {
                adjustVolume(10);
                showVolumeIndicator();
            }
            else if (keyEvent->code == sf::Keyboard::Key::Down && currentState == PLAYING) {
                adjustVolume(-10);
                showVolumeIndicator();
            }

            // --- Обработка в зависимости от состояния ---
            switch (currentState) {
            case MENU:
                handleMenuEvents(*keyEvent);
                break;

            case SETTINGS:
                handleSettingsEvents(*keyEvent);
                break;

            case PLAYING:
                handlePlayingEvents(*keyEvent);
                break;

            case ABOUT:
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    currentState = MENU;
                    window.setMouseCursorVisible(true);
                }
                break;

            case GAMEOVER:
                // Любая клавиша → выход в главное меню
                returnToMenu();
                break;
            }
        }

        // 3. Движение мыши (навигация в меню и настройках)
        else if (const auto* mouseMove = event->getIf<sf::Event::MouseMoved>()) {
            if (currentState == MENU) {
                int item = getMenuItemAt(mouseMove->position.x, mouseMove->position.y);
                if (item != -1) menuSelection = item;
            }
            else if (currentState == SETTINGS) {
                int item = getSettingsItemAt(mouseMove->position.x, mouseMove->position.y);
                if (item != -1) settingsSelection = item;
            }
        }

        // 4. Клик левой кнопкой мыши
        else if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseBtn->button == sf::Mouse::Button::Left) {
                switch (currentState) {
                case MENU: {
                    int item = getMenuItemAt(mouseBtn->position.x, mouseBtn->position.y);
                    if (item == 0) {          // Старт
                        startGame();          // новый метод – запуск игры
                    }
                    else if (item == 1) {   // Настройки
                        currentState = SETTINGS;
                        settingsSelection = 0;
                        window.setMouseCursorVisible(true);
                    }
                    else if (item == 2) {   // О программе
                        currentState = ABOUT;
                        window.setMouseCursorVisible(true);
                    }
                    break;
                }

                case SETTINGS: {
                    // Для простоты только переключение подсказок по клику
                    int item = getSettingsItemAt(mouseBtn->position.x, mouseBtn->position.y);
                    if (item == 2) {
                        showTooltips = !showTooltips;
                    }
                    // Изменение громкости/сложности мышкой не реализовано (используйте клавиши)
                    break;
                }

                case PLAYING:
                    // Стрельба только если игра активна и не на паузе и не завершена
                    if (!paused && !gameFinished) {
                        handleShot(mouseBtn->position.x, mouseBtn->position.y);
                    }
                    break;

                case GAMEOVER:
                    // Клик мыши в состоянии завершения → выход в меню
                    returnToMenu();
                    break;

                default:
                    break;
                }
            }
        }
    }
}

void Game::handleMenuEvents(const sf::Event::KeyPressed& key) {
    if (key.code == sf::Keyboard::Key::Up) {
        menuSelection = (menuSelection - 1 + 3) % 3;
    }
    else if (key.code == sf::Keyboard::Key::Down) {
        menuSelection = (menuSelection + 1) % 3;
    }
    else if (key.code == sf::Keyboard::Key::Enter) {
        if (menuSelection == 0) { // Старт
            startGame();
        }
        else if (menuSelection == 1) { // Настройки
            currentState = SETTINGS;
            settingsSelection = 0;
        }
        else if (menuSelection == 2) { // О программе
            currentState = ABOUT;
        }
    }
}

void Game::handleSettingsEvents(const sf::Event::KeyPressed& key) {
    if (key.code == sf::Keyboard::Key::Up) {
        settingsSelection = (settingsSelection - 1 + 3) % 3;
    }
    else if (key.code == sf::Keyboard::Key::Down) {
        settingsSelection = (settingsSelection + 1) % 3;
    }
    else if (key.code == sf::Keyboard::Key::Left) {
        if (settingsSelection == 0) { // громкость
            tempVolume = max(0, tempVolume - 5);
            soundManager.setVolume(tempVolume); // сразу применяем для теста
            showVolumeIndicator();
        }
        else if (settingsSelection == 1) { // сложность
            tempDifficulty = max(1, tempDifficulty - 1);
        }
        else if (settingsSelection == 2) { // подсказки
            showTooltips = !showTooltips;
        }
    }
    else if (key.code == sf::Keyboard::Key::Right) {
        if (settingsSelection == 0) {
            tempVolume = min(100, tempVolume + 5);
            soundManager.setVolume(tempVolume);
            showVolumeIndicator();
        }
        else if (settingsSelection == 1) {
            tempDifficulty = min(3, tempDifficulty + 1);
        }
        else if (settingsSelection == 2) {
            showTooltips = !showTooltips;
        }
    }
    else if (key.code == sf::Keyboard::Key::Escape) {
        // Сохранить настройки и вернуться в меню
        config.volume = tempVolume;
        config.difficulty = tempDifficulty;
        SettingsManager::saveConfig(config);
        currentState = MENU;
        window.setMouseCursorVisible(true);
    }
}

void Game::handlePlayingEvents(const sf::Event::KeyPressed& key) {
    if (key.code == sf::Keyboard::Key::P || key.code == sf::Keyboard::Key::Space) {
        paused = !paused;
    }
    else if (key.code == sf::Keyboard::Key::F1) {
        changeDifficulty(1);
        tempDifficulty = 1;
    }
    else if (key.code == sf::Keyboard::Key::F2) {
        changeDifficulty(2);
        tempDifficulty = 2;
    }
    else if (key.code == sf::Keyboard::Key::F3) {
        changeDifficulty(3);
        tempDifficulty = 3;
    }
    else if (key.code == sf::Keyboard::Key::Escape) {
        returnToMenu();
    }
}

void Game::adjustVolume(int delta) {
    int newVol = soundManager.getVolume() + delta;
    if (newVol < 0) newVol = 0;
    if (newVol > 100) newVol = 100;
    soundManager.setVolume(newVol);
    tempVolume = newVol;
    config.volume = newVol;
}

void Game::showVolumeIndicator() {
    if (volumeIndicator.has_value()) {
        volumeIndicator->setString(L"Громкость: " + std::to_wstring(soundManager.getVolume()) + L"%");
        volumeIndicatorTimer = 1.0f; // показывать 1 секунду
    }
}

void Game::updateSpawn() {
    frameCounter++;
    if (frameCounter >= spawnDelayFrames) {
        frameCounter = 0;
        spawnTarget();
    }
}

void Game::spawnTarget() {
    int x = rand() % (window.getSize().x - Target::WIDTH);
    int y = 50;
    targets.emplace_back(targetTexture, x, y);
}

void Game::updateTargets() {
    const float speed = 2.0f;
    for (auto& t : targets) t.move(0, speed);
    targets.erase(std::remove_if(targets.begin(), targets.end(),
        [this](const Target& t) { return t.getY() > window.getSize().y; }),
        targets.end());
}

void Game::handleShot(int mouseX, int mouseY) {
    if (gameFinished) return; // если игра завершена, стрельба не обрабатывается
    bool hit = false;
    for (auto it = targets.begin(); it != targets.end(); ) {
        if (it->contains(mouseX, mouseY)) {
            hit = true;
            score.addPoint(10);
            soundManager.playHit();
            it = targets.erase(it);
        }
        else ++it;
    }
    if (!hit) soundManager.playShoot();

    // Проверка победы после каждого попадания
    if (timedMode && score.getScore() >= targetScore) {
        gameFinished = true;
        currentState = GAMEOVER;
        gameOverTimer = 5.f;
        gameOverClock.restart();
    }
}

void Game::render() {
    window.clear();
    if (currentState == MENU) renderMenu();
    else if (currentState == SETTINGS) renderSettings();
    else if (currentState == ABOUT) renderAbout();
    else if (currentState == PLAYING) renderGame();
    else if (currentState == GAMEOVER) renderGameOver();

    // Индикатор громкости рисуется поверх всего, если активен
    if (volumeIndicator.has_value() && volumeIndicatorTimer > 0.f && volumeIndicator->getString() != L"") {
        window.draw(*volumeIndicator);
    }
    window.display();
}

void Game::renderMenu() {
    window.draw(backgroundSprite);
    std::vector<std::wstring> items = { L"Старт", L"Настройки", L"О программе" };
    float startY = 200.f;
    float startX = 350.f;
    for (int i = 0; i < 3; ++i) {
        sf::Text item(font, items[i], 28);
        item.setPosition({ startX, startY + i * 40.f });
        if (i == menuSelection) {
            item.setFillColor(sf::Color::Yellow);

            item.setString(sf::String(L"> " + items[i]));
        }
        else {
            item.setFillColor(sf::Color::White);
            item.setString(L"  " + items[i]);
        }
        window.draw(item);
    }
}

void Game::renderSettings() {
    window.draw(backgroundSprite);
    std::vector<std::wstring> labels = { L"Громкость: " + std::to_wstring(tempVolume),
                                       L"Сложность: " + std::to_wstring(tempDifficulty) + L" (1-Легкий,2-Нормальный,3-Сложный)",
                                       L"Подсказки: " + std::wstring(showTooltips ? L"Вкл" : L"Выкл") };
    float startY = 150.f;
    float startX = 150.f;
    for (int i = 0; i < 3; ++i) {
        sf::Text item(font, labels[i], 24);
        item.setPosition({ startX, startY + i * 40.f });
        if (i == settingsSelection) {
            item.setFillColor(sf::Color::Yellow);
            item.setString(L"-> " + labels[i]);
        }
        else {
            item.setFillColor(sf::Color::White);
            item.setString(L"   " + labels[i]);
        }
        window.draw(item);
    }
    sf::Text hint(font, L"LEFT/RIGHT для изменения | ESC сохранение", 18);
    hint.setFillColor(sf::Color::White);
    hint.setPosition({ 150.f, 300.f });
    window.draw(hint);
}

void Game::renderAbout() {
    window.draw(backgroundSprite);
    window.draw(*aboutText);
    sf::Text backText(font, L"ESC для выходв", 18);
    backText.setFillColor(sf::Color::White);
    backText.setPosition({ 10.f, 550.f });
    window.draw(backText);
}

void Game::renderGame() {
    window.draw(backgroundSprite);
    for (auto& t : targets) t.draw(window);
    score.draw(window);
    if (difficultyText.has_value()) window.draw(*difficultyText);
    if (showTooltips) renderTooltips();

    // Отображение таймера и цели
    if (timedMode) {
        if (timerText.has_value()) {
            timerText->setString(L"Время: " + std::to_wstring((int)max(0.f, timeRemaining)) + L" с");
            window.draw(*timerText);
        }
        if (targetText.has_value()) {
            targetText->setString(L"Цель: " + std::to_wstring(targetScore) + L" очков");
            window.draw(*targetText);
        }
    }

    // Прицел
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    crosshairSprite.setPosition({ (float)mousePos.x - 16, (float)mousePos.y - 16 });
    window.draw(crosshairSprite);

    if (paused && pauseText.has_value()) window.draw(*pauseText);

    // Если игра завершена – показать сообщение и выйти в меню (можно сделать через состояние)
    /*if (gameFinished) {
        sf::Text gameOverText(font,
            (score.getScore() >= targetScore) ? L"ПОБЕДА!" : L"ПОРАЖЕНИЕ!",
            50);
        gameOverText.setFillColor((score.getScore() >= targetScore) ? sf::Color::Green : sf::Color::Red);
        gameOverText.setPosition({ 300.f, 300.f });
        window.draw(gameOverText);
        renderGameOver();
    }*/
}

void Game::renderTooltips() {
    sf::Text tips(font, L"F1/F2/F3: Сложность | Up/Down: Громкость | P/Space: Пауза | Esc: Меню" , 16);
    tips.setFillColor(sf::Color::White);
    tips.setPosition({ 10.f, 570.f });
    window.draw(tips);
}

void Game::updateDifficultyDisplay() {
    if (difficultyText.has_value()) {
        std::wstring diffStr;
        switch (tempDifficulty) {
        case 1: diffStr = L"Сложность: Легкий (F1)"; break;
        case 2: diffStr = L"Сложность: Обычный (F2)"; break;
        case 3: diffStr = L"Сложность: Сложный (F3)"; break;
        }
        difficultyText->setString(diffStr);
    }
}

void Game::changeDifficulty(int newDifficulty) {
    if (newDifficulty < 1) newDifficulty = 1;
    if (newDifficulty > 3) newDifficulty = 3;
    tempDifficulty = newDifficulty;
    switch (tempDifficulty) {
    case 1: spawnDelayFrames = 80; break;
    case 2: spawnDelayFrames = 45; break;
    case 3: spawnDelayFrames = 20; break;
    }
    updateDifficultyDisplay();
}

void Game::returnToMenu() {
    currentState = MENU;
    window.setMouseCursorVisible(true);
    paused = false;
    gameOverClock.restart();
    // Сохраняем настройки (опционально, можно оставить на закрытие)
    // но лучше сохранять при явном выходе
    config.volume = tempVolume;
    config.difficulty = tempDifficulty;
    std::string basePath = getExecutablePath();
    SettingsManager::saveConfig(config, basePath + "settings.ini");
}

void Game::resetGame() {
    score.reset();
    targets.clear();
    frameCounter = 0;
    gameFinished = false;
    if (timedMode) {
        timeRemaining = 60.f;
        timeRemaining = static_cast<float>(timeLimit); // ← берем из timeLimit
        gameClock.restart();
    }
}

// REMOVEME
void Game::checkGameEnd() {
    if (gameFinished || !timedMode) return;

    if (score.getScore() >= targetScore) {
        // Победа
        gameFinished = true;
        currentState = GAMEOVER;
        gameOverTimer = 5.f;
        gameOverClock.restart();
    }
}

void Game::startGame() {
    currentState = PLAYING;
    paused = false;
    window.setMouseCursorVisible(false);
    resetGame();
    // Применяем текущие настройки сложности и громкости
    soundManager.setVolume(tempVolume);
    changeDifficulty(tempDifficulty);
}

void Game::renderGameOver() {
    window.draw(backgroundSprite);
    sf::Text message(font,
        (score.getScore() >= targetScore) ? L"ПОБЕДА!" : L"ПОРАЖЕНИЕ!",
        60);
    message.setFillColor((score.getScore() >= targetScore) ? sf::Color::Green : sf::Color::Red);
    message.setPosition({ 200.f, 250.f });
    window.draw(message);

    sf::Text info(font, L"Набрано очков: " + std::to_wstring(score.getScore()) +
        L" / " + std::to_wstring(targetScore), 24);
    info.setFillColor(sf::Color::White);
    info.setPosition({ 280.f, 330.f });
    window.draw(info);

    sf::Text tips(font, L"Нажмите Esc для выхода в главное меню", 16);
    tips.setFillColor(sf::Color::White);
    tips.setPosition({ 10.f, 570.f });
    window.draw(tips); 
}