#include "Entity.hpp"

void Entity::drawHealthBar(sf::RenderTarget& target, float yOffset){
    if(m_hp >= m_maxHp) return;

    float barWidth = 40.0f;
    float barHeight = 5.0f;

    sf::RectangleShape bg({barWidth, barHeight});
    bg.setFillColor(sf::Color::Red);
    bg.setOrigin({barWidth / 2.0f, barHeight / 2.0f});
    bg.setPosition({m_position.x, m_position.y - yOffset});

    float hpPercent = std::max(0.0f, m_hp / m_maxHp);
    sf::RectangleShape fg({barWidth * hpPercent * 0.95f, barHeight * 0.95f});
    fg.setFillColor(sf::Color::Green);
    fg.setOrigin({barWidth / 2.0f, barHeight / 2.0f});
    fg.setPosition({m_position.x, m_position.y - yOffset});

    target.draw(bg);
    target.draw(fg);
}