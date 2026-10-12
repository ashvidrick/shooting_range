#include "Target.h"

Target::Target(const sf::Texture& texture, float x, float y)
    : sprite(texture)   // инициализация sprite сразу с текстурой
{
    sprite.setPosition({ x, y });
}

void Target::draw(sf::RenderWindow& window) const {
    window.draw(sprite);
}

bool Target::contains(int mouseX, int mouseY) const {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    return bounds.contains({ static_cast<float>(mouseX), static_cast<float>(mouseY) });
}

void Target::move(float dx, float dy) {
    sprite.move({ dx, dy });
}