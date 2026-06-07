#include "Enemy.hpp"

/**
 * @brief Constructs an enemy with registry-defined stats and appearance.
 * @param id Unique network entity identifier.
 * @param startPos Initial world position.
 * @param type Enemy archetype used to look up stats and color.
 */
Enemy::Enemy(std::uint32_t id, const sf::Vector2f& startPos, EnemyType type) : Entity(id, startPos), m_type(type){
    const auto& stats = EnemyRegistry::getStats(type);
    m_maxHp = stats.maxHp;
    m_hp = stats.maxHp;
    m_shape.setRadius(stats.radius);
    m_shape.setFillColor(stats.color);
    m_shape.setOrigin({stats.radius, stats.radius});
    m_shape.setPosition(m_position);
}

/**
 * @brief Syncs the drawable shape to the current entity position.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map reference.
 */
void Enemy::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    m_shape.setPosition(m_position);
}

/**
 * @brief Draws the enemy shape and its health bar.
 * @param target Render target to draw into.
 */
void Enemy::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
    drawHealthBar(target, 30.0f);
}
