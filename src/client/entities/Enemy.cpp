#include "Enemy.hpp"

Enemy::Enemy(std::uint32_t id, const sf::Vector2f& startPos, EnemyType type) : Entity(id, startPos), m_type(type){
    const auto& stats = EnemyRegistry::getStats(type);
    m_maxHp = stats.maxHp;
    m_hp = stats.maxHp;
    m_shape.setRadius(stats.radius);
    m_shape.setFillColor(stats.color);
    m_shape.setOrigin({stats.radius, stats.radius});
    m_shape.setPosition(m_position);
}

void Enemy::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    m_shape.setPosition(m_position);
}

void Enemy::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
    drawHealthBar(target, 30.0f);
}
