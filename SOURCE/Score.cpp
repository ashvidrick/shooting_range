#include "Score.h"
#include "tools.h"
#include <stdexcept>
#include <windows.h> 

// move to utils.h or insert inplace if possible

Score::Score() : score(0) 
{
    std::string basePath = getExecutablePath();
    // Загрузка шрифта для текста (например, Arial из системной папки)
    if (!font.openFromFile(basePath + "res/arial.ttf")) {
        // Если не загрузился, используем встроенный шрифт SFML (но его нет, можно заглушку)
        // В реальном проекте можно положить свой шрифт в папку res/
        throw std::runtime_error("Failed to load font" + basePath + "res/arial.ttf");
    }
    text.emplace(font, "Score: 0", 30);
    text.value().setFillColor(sf::Color::Yellow);
    text.value().setPosition({ 10.f, 10.f });
    updateText();
}

void Score::addPoint(int points) {
    score += points;
    updateText();
}

void Score::draw(sf::RenderWindow& window) const {
    window.draw(text.value());
}

void Score::updateText() {
    text.value().setString("Score: " + std::to_string(score));
}
