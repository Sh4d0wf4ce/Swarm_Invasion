#include "AcidProjectile.hpp"

AcidProjectile::AcidProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::AcidSpit) {
    m_lifetime = 4.0f;
    m_shape.setRadius(m_radius);
    m_shape.setFillColor(WeaponRegistry::getStats(m_weaponType).color);
    m_shape.setOrigin({m_radius, m_radius});
}

void AcidProjectile::update(sf::Time deltaTime) {
    m_position += m_velocity * deltaTime.asSeconds();
    m_lifetime -= deltaTime.asSeconds();
    if(m_lifetime <= 0.0f) m_active = false;
}

void AcidProjectile::render(sf::RenderTarget& target) {
    m_shape.setPosition(m_position);
    target.draw(m_shape);
}