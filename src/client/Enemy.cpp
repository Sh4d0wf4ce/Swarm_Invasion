#include "Enemy.hpp"

Enemy::Enemy(std::uint32_t id, const sf::Vector2f& startPos) : Entity(id, startPos){
    m_maxHp = Config::ENEMY_MAX_HP;
    m_hp = Config::ENEMY_MAX_HP;

    m_shape.setRadius(Config::ENEMY_RADIUS);
    m_shape.setFillColor(sf::Color::Green);
    m_shape.setOrigin({Config::ENEMY_RADIUS, Config::ENEMY_RADIUS});
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