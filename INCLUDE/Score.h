#pragma once

#include <SFML/Graphics.hpp>

class Score {
public:
    Score();
    void addPoint(int points);
    void reset() { score = 0; updateText(); }
    void draw(sf::RenderWindow& window) const;
    int getScore() const { return score; }

private:
    int score;
    sf::Font font;
    std::optional < sf::Text > text;
    void updateText();
};
