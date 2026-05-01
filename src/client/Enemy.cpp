#include "Enemy.hpp"

Enemy::Enemy(std::uint32_t id, const sf::Vector2f& startPos) : Entity(id, startPos){
    m_shape.setRadius(15.0f);
    m_shape.setFillColor(sf::Color::Green);
    m_shape.setOrigin({15.0f, 15.0f});
    m_shape.setPosition(m_position);
}

void Enemy::update(sf::Time deltaTime){
    
}

void Enemy::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}