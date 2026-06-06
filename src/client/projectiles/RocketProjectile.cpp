#include "RocketProjectile.hpp"
#include "../entities/Entity.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <cmath>

RocketProjectile::RocketProjectile(std::uint32_t ownerId, const sf::Vector2f& startPos, const sf::Vector2f& velocity, Faction faction)
    : Projectile(ownerId, startPos, velocity, faction, WeaponType::Rocket) {
    m_lifetime = 5.0f;

    const auto& stats = WeaponRegistry::getStats(WeaponType::Rocket);
    if (stats.explosionRadius > 0.0f) {
        m_explosionRadius = stats.explosionRadius;
    }

    static sf::Texture rocketTex;
    static bool texLoaded = false;
    if (!texLoaded) {
        if (rocketTex.loadFromFile("assets/textures/rocket.png")) {
            rocketTex.setSmooth(true);
        }
        texLoaded = true;
    }

    m_sprite.emplace(rocketTex);
    m_sprite->setOrigin({32.0f, 32.0f});

    m_explosionShape.setFillColor(sf::Color(255, 120, 40, 0));
    m_explosionShape.setOutlineThickness(2.0f);
    m_explosionShape.setOutlineColor(sf::Color(255, 200, 80, 0));
}

void RocketProjectile::update(sf::Time deltaTime) {
    if (m_isExploding) {
        m_explosionTimer += deltaTime.asSeconds();
        if (m_explosionTimer >= m_maxExplosionTime) {
            m_active = false;
        }
        return;
    }

    Projectile::update(deltaTime);
}

void RocketProjectile::render(sf::RenderTarget& target) {
    if (m_isExploding) {
        float progress = m_explosionTimer / m_maxExplosionTime;
        if (progress > 1.0f) return;

        float easeOut = 1.0f - std::pow(1.0f - progress, 3.0f);
        float currentRadius = m_explosionRadius * easeOut;

        m_explosionShape.setPosition(m_explosionPos);
        m_explosionShape.setRadius(currentRadius);
        m_explosionShape.setOrigin({currentRadius, currentRadius});

        std::uint8_t alpha = static_cast<std::uint8_t>(200.0f * (1.0f - progress));
        m_explosionShape.setFillColor(sf::Color(255, 120, 40, alpha));
        m_explosionShape.setOutlineColor(sf::Color(255, 200, 80, static_cast<std::uint8_t>(alpha + 30 > 255 ? 255 : alpha + 30)));

        target.draw(m_explosionShape);
        return;
    }

    if (!m_sprite) return;

    float angleDeg = std::atan2(m_velocity.y, m_velocity.x) * (180.0f / static_cast<float>(M_PI));
    m_sprite->setPosition(m_position);
    m_sprite->setRotation(sf::degrees(angleDeg));
    target.draw(*m_sprite);
}

std::vector<std::uint32_t> RocketProjectile::checkCollisions(const std::vector<Entity*>& entities, const std::shared_ptr<MapGenerator>& map) {
    std::vector<std::uint32_t> hitlist;
    if (!m_active || m_isExploding) return hitlist;

    bool detonated = false;

    if (map) {
        int gridX = static_cast<int>(m_position.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(m_position.y / Config::TILE_SIZE);
        if (map->getTile(gridX, gridY) == TileType::Wall) detonated = true;
    }

    if (!detonated) {
        for (Entity* entity : entities) {
            if (entity->getFaction() == m_faction) continue;
            if (entity->getId() == m_ownerId) continue;

            sf::Vector2f diff = m_position - entity->getPosition();
            float collDist = entity->getRadius() + m_radius;
            if (diff.lengthSquared() < collDist * collDist) {
                detonated = true;
                break;
            }
        }
    }

    if (detonated) {
        m_explosionPos = m_position;
        m_isExploding = true;
        m_explosionTimer = 0.0f;

        for (Entity* entity : entities) {
            if (entity->getFaction() == m_faction) continue;
            if (entity->getId() == m_ownerId) continue;

            sf::Vector2f diff = m_explosionPos - entity->getPosition();
            if (diff.lengthSquared() <= m_explosionRadius * m_explosionRadius) {
                hitlist.push_back(entity->getId());
            }
        }
    }

    return hitlist;
}
