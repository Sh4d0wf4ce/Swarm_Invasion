#include "NeedleProjectile.hpp"
#include "../entities/Entity.hpp"
#include <cmath>

/**
 * @brief Builds the needle triangle shape and applies medic color styling.
 * @param ownerId Entity ID of the shooter.
 * @param startPos World position where the needle spawns.
 * @param velocity Initial movement vector in pixels per second.
 * @param faction Faction of the owner.
 */
NeedleProjectile::NeedleProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::MedicNeedle) {
    m_needleShape.setPointCount(3);
    m_needleShape.setPoint(0, {14.0f, 0.0f});
    m_needleShape.setPoint(1, {-8.0f, -4.0f});
    m_needleShape.setPoint(2, {-8.0f, 4.0f});
    m_needleShape.setFillColor(sf::Color(0, 255, 180, 230));
    m_needleShape.setOutlineColor(sf::Color(100, 255, 255, 255));
    m_needleShape.setOutlineThickness(1.5f);
    m_needleShape.setOrigin({0.0f, 0.0f});
}

/**
 * @brief Draws the needle shape aligned to its velocity direction.
 * @param target Render target to draw into.
 */
void NeedleProjectile::render(sf::RenderTarget& target) {
    float angle = std::atan2(m_velocity.y, m_velocity.x) * (180.0f / static_cast<float>(M_PI));
    m_needleShape.setPosition(m_position);
    m_needleShape.setRotation(sf::degrees(angle));
    target.draw(m_needleShape);
}

/**
 * @brief Heals player allies or damages enemies on contact; deactivates after one hit or wall impact.
 * @param entities Candidate entities to test against.
 * @param map Map used for wall tile checks.
 * @return ID of the entity hit, if any.
 */
std::vector<std::uint32_t> NeedleProjectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) {
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

    // --- Heal allies or damage enemies on contact ---
    for (Entity* entity : entities) {
        if (entity->getId() == m_ownerId) continue;

        if (entity->getFaction() == Faction::Players) {
            sf::Vector2f diff = m_position - entity->getPosition();
            float collDist = entity->getRadius() + m_radius;
            if (diff.lengthSquared() < collDist * collDist) {
                hitlist.push_back(entity->getId());
                m_active = false;
                break;
            }
        } else if (entity->getFaction() == Faction::Enemies) {
            sf::Vector2f diff = m_position - entity->getPosition();
            float collDist = entity->getRadius() + m_radius;
            if (diff.lengthSquared() < collDist * collDist) {
                hitlist.push_back(entity->getId());
                m_active = false;
                break;
            }
        }
    }

    return hitlist;
}
