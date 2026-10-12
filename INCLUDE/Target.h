#pragma once

#include <SFML/Graphics.hpp>

class Target {
public:
    static constexpr int WIDTH = 64;
    static constexpr int HEIGHT = 64;

    Target(const sf::Texture& texture, float x, float y);

    void draw(sf::RenderWindow& window) const;
    bool contains(int mouseX, int mouseY) const;
    void move(float dx, float dy);
    float getY() const { return sprite.getPosition().y; }

private:
    sf::Sprite sprite;
};
