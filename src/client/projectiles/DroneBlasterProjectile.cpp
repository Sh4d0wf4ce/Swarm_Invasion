#include "DroneBlasterProjectile.hpp"
#include "../entities/Entity.hpp"
#include <cmath>
#include <algorithm>

/**
 * @brief Initializes the beam rectangle shape and styling.
 * @param ownerId Entity ID of the medic drone owner.
 * @param startPos World position where the beam spawns.
 * @param velocity Initial movement vector in pixels per second.
 * @param faction Faction of the owner.
 */
DroneBlasterProjectile::DroneBlasterProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::DroneBlaster) {
        
    m_beamShape.setSize({22.0f, 5.0f});
    m_beamShape.setOrigin({0.0f, 2.5f});
    m_beamShape.setFillColor(sf::Color(255, 40, 40, 230));
    m_beamShape.setOutlineColor(sf::Color(255, 180, 80, 200));
    m_beamShape.setOutlineThickness(1.0f);
}

/**
 * @brief Draws the beam rectangle aligned to its velocity direction.
 * @param target Render target to draw into.
 */
void DroneBlasterProjectile::render(sf::RenderTarget& target) {
    float angle = std::atan2(m_velocity.y, m_velocity.x) * (180.0f / static_cast<float>(M_PI));
    m_beamShape.setPosition(m_position);
    m_beamShape.setRotation(sf::degrees(angle));
    target.draw(m_beamShape);
}

/**
 * @brief Tracks unique enemy contacts without deactivating until lifetime ends or a wall is hit.
 * @param entities Candidate entities to test against.
 * @param map Map used for wall tile checks.
 * @return Empty list; hits are tracked internally for server-authoritative damage.
 */
std::vector<std::uint32_t> DroneBlasterProjectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) {
    std::vector<std::uint32_t> hitlist;
    if (!m_active) return hitlist;

    // --- Deactivate on wall impact ---
    if (map) {
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if (map->getTile(gridX, gridY) == TileType::Wall) {
            m_active = false;
            return hitlist;
        }
    }

    // --- Track unique enemy hits without deactivating early ---
    for (Entity* entity : entities) {
        if (entity->getFaction() != Faction::Enemies) continue;

        std::uint32_t id = entity->getId();
        if (std::find(m_hitEnemies.begin(), m_hitEnemies.end(), id) != m_hitEnemies.end()) continue;

        sf::Vector2f diff = m_position - entity->getPosition();
        float collDist = entity->getRadius() + m_radius;
        if (diff.lengthSquared() < collDist * collDist) {
            m_hitEnemies.push_back(id);
        }
    }

    if (m_lifetime <= 0.0f) m_active = false;
    return hitlist;
}
