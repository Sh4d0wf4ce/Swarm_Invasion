#include "WaveProjectile.hpp"
#include <cmath>
#include <algorithm>

/**
 * @brief Initializes wave direction and mesh primitive type for arc rendering.
 * @param ownerId Entity ID of the shooter.
 * @param startPos World position where the wave spawns.
 * @param velocity Initial movement vector used to derive facing angle.
 * @param faction Faction of the owner.
 */
WaveProjectile::WaveProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::VanguardWave) {

    // --- Compute wave direction for arc rendering ---
    m_facingAngle = std::atan2(velocity.y, velocity.x);
    m_mesh.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
}

/**
 * @brief Moves the wave and rebuilds its tapered arc mesh each frame.
 * @param deltaTime Elapsed time since the last update.
 */
void WaveProjectile::update(sf::Time deltaTime) {
    Projectile::update(deltaTime);

    // --- Rebuild tapered arc mesh each frame ---
    m_mesh.clear();
    int segments = 30; 
    float span = 120.0f * (3.14159f / 180.0f); 
    float startAngle = m_facingAngle - span / 2.0f;
    float visualRadius = 95.0f; 
    
    sf::Vector2f faceDir(std::cos(m_facingAngle), std::sin(m_facingAngle));
    sf::Vector2f visualCenter = m_position - faceDir * (visualRadius - m_radius);

    for(int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        float angle = startAngle + t * span;
        float taper = std::sin(t * 3.14159f); 
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        sf::Vector2f outer = visualCenter + dir * visualRadius;
        float thickness = 12.0f * taper;
        sf::Vector2f inner = visualCenter + dir * (visualRadius - thickness) - faceDir * (4.0f * taper);
        std::uint8_t alpha = static_cast<std::uint8_t>(255.0f * std::clamp(taper * 1.5f, 0.0f, 1.0f));
        m_mesh.append(sf::Vertex(inner, sf::Color(255, 255, 255, alpha)));
        m_mesh.append(sf::Vertex(outer, sf::Color(0, 255, 255, alpha)));
    }
}

/**
 * @brief Records first contact with each hostile entity without deactivating the wave.
 * @param entities Candidate entities to test against.
 * @param map Unused for wave collision; wall checks are skipped.
 * @return IDs of newly hit entities this frame.
 */
std::vector<std::uint32_t> WaveProjectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) {
    std::vector<std::uint32_t> hitlist;
    if(!m_active) return hitlist;

    // --- Record first hit per entity without deactivating ---
    for(Entity* entity : entities) {
        if(entity->getFaction() == m_faction) continue;
        if(entity->getId() == m_ownerId) continue;
        if(m_hitEntities.count(entity->getId())) continue;
        sf::Vector2f diff = m_position - entity->getPosition();
        float collDist = entity->getRadius() + m_radius;
        if(diff.lengthSquared() < collDist * collDist){
            hitlist.push_back(entity->getId());
            m_hitEntities.insert(entity->getId());
        }
    }
    return hitlist;
}

/**
 * @brief Draws the wave arc vertex mesh.
 * @param target Render target to draw into.
 */
void WaveProjectile::render(sf::RenderTarget& target) {
    target.draw(m_mesh);
}
