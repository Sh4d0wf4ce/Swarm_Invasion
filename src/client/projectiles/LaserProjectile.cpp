#include "LaserProjectile.hpp"

LaserProjectile::LaserProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Laser) {
    
    m_lifetime = 1.5f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void LaserProjectile::update(sf::Time deltaTime){
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();

    if(m_lifetime <= 0.0f) m_active = false;
}

void LaserProjectile::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}