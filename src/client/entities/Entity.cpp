#include "Entity.hpp"

void Entity::drawHealthBar(sf::RenderTarget& target, float yOffset){
    if(m_hp >= m_maxHp) return;

    // --- Draw red background bar ---
    float barWidth = 40.0f;
    float barHeight = 5.0f;
    sf::RectangleShape bg({barWidth, barHeight});
    bg.setFillColor(sf::Color::Red);
    bg.setOrigin({barWidth / 2.0f, barHeight / 2.0f});
    bg.setPosition({m_position.x, m_position.y - yOffset});

    // --- Draw green foreground proportional to remaining HP ---
    float hpPercent = std::max(0.0f, m_hp / m_maxHp);
    sf::RectangleShape fg({barWidth * hpPercent, barHeight});
    fg.setFillColor(sf::Color::Green);
    fg.setOrigin({barWidth / 2.0f, barHeight / 2.0f});
    fg.setPosition({m_position.x, m_position.y - yOffset});
    target.draw(bg);
    target.draw(fg);
}
