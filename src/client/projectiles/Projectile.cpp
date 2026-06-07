#include "Projectile.hpp"

/**
 * @brief Constructs a projectile from weapon stats and initializes its visual shape.
 * @param ownerId Entity ID of the shooter.
 * @param startPos World position where the projectile spawns.
 * @param velocity Initial movement vector in pixels per second.
 * @param faction Faction of the owner; used to filter collision targets.
 * @param weapon Weapon type used to look up radius, lifetime, and color.
 */
Projectile::Projectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity,  Faction faction, WeaponType weapon)
    : m_ownerId(ownerId), m_position(startPos), m_velocity(velocity), m_faction(faction), m_weaponType(weapon)  {
        const auto& stats = WeaponRegistry::getStats(weapon);
        m_radius = stats.radius;
        m_lifetime = stats.lifetime;
        m_active = true;
        m_shape.setRadius(m_radius);
        m_shape.setFillColor(stats.color);
        m_shape.setOrigin({m_radius, m_radius});
}

/**
 * @brief Tests wall and entity collisions and deactivates on first hostile hit.
 * @param entities Candidate entities to test against.
 * @param map Map used for wall tile checks; may be null to skip wall collision.
 * @return IDs of entities hit; at most one ID for the base projectile.
 */
std::vector<std::uint32_t> Projectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map){
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    // --- Check wall collision and deactivate on impact ---
    if(map){
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if(map->getTile(gridX, gridY) == TileType::Wall){
            m_active = false;
            return hitlist;
        }
    }

    // --- Check entity collision and record first hit ---
    for(Entity* entity : entities) {
        if(entity->getFaction() == m_faction) continue;
        if(entity->getId() == m_ownerId) continue;

        sf::Vector2f diff = m_position - entity->getPosition();
        float collDist = entity->getRadius() + m_radius;
        
        if(diff.lengthSquared() < collDist * collDist){
            hitlist.push_back(entity->getId());
            m_active = false;
            break;
        }
    }
    return hitlist;
}

/**
 * @brief Advances position and decrements lifetime; deactivates when expired.
 * @param deltaTime Elapsed time since the last update.
 */
void Projectile::update(sf::Time deltaTime){
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();
    if(m_lifetime <= 0.0f) m_active = false;
}

/**
 * @brief Draws the projectile circle shape at its current position.
 * @param target Render target to draw into.
 */
void Projectile::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}
