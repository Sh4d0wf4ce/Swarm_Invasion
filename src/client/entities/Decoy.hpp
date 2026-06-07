#pragma once
#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "AbilityRegistry.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

class Decoy : public Entity {
public:
    Decoy(std::uint32_t id, const sf::Vector2f& startPos, float maxHp = -1.f)
        : Entity(id, startPos) {
        if (maxHp < 0.f) {
            maxHp = AbilityRegistry::param(AbilityRegistry::vanguard().Decoy, "decoyHp", 150.f);
        }
        m_maxHp = maxHp;
        m_hp = maxHp;
        auto& stats = HeroRegistry::getStats(PlayerClass::Vanguard);
        m_shape.setRadius(stats.radius);
        m_shape.setOrigin({stats.radius, stats.radius});
        m_shape.setPosition(startPos);
        m_shape.setFillColor(stats.color);
        m_shape.setOutlineColor(sf::Color(255, 255, 255, 200));
        m_shape.setOutlineThickness(1.5f);
        m_explosionShape.setPosition(startPos);
        m_explosionShape.setFillColor(sf::Color(0, 255, 255, 0));
    }


    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
        (void)map;
        m_shape.setPosition(m_position);
        m_timeAlive += deltaTime.asSeconds();
        float pulse = std::abs(std::sin(m_timeAlive * 15.0f));
        m_shape.setOutlineThickness(1.0f + 2.0f * pulse);
        if (m_isExploding) {
            m_explosionTimer += deltaTime.asSeconds();
            float progress = m_explosionTimer / m_maxExplosionTime;
            if (progress <= 1.0f) {
                float easeOut = 1.0f - std::pow(1.0f - progress, 3.0f);
                float currentRadius = AbilityRegistry::param(AbilityRegistry::vanguard().Decoy, "explosionRadius", 120.f) * easeOut;
                m_explosionShape.setPosition(m_position);
                m_explosionShape.setRadius(currentRadius);
                m_explosionShape.setOrigin({currentRadius, currentRadius});
                std::uint8_t alpha = static_cast<std::uint8_t>(180.0f * (1.0f - progress));
                m_explosionShape.setFillColor(sf::Color(0, 255, 255, alpha));
            }
        }
    }

    void render(sf::RenderTarget& target) override {
        if (!m_isExploding) {
            target.draw(m_shape);
            drawHealthBar(target, 30.0f);
        } else {
            target.draw(m_explosionShape);
        }
    }

    // Explosion Control
    void triggerExplosion() { m_isExploding = true; }
    bool isExploding() const { return m_isExploding; }
    bool isFinished() const { return m_isExploding && (m_explosionTimer >= m_maxExplosionTime); }

    Faction getFaction() const override { return Faction::Players; }
    float getRadius() const override { return HeroRegistry::getStats(PlayerClass::Vanguard).radius; }

private:
    sf::CircleShape m_shape;
    sf::CircleShape m_explosionShape;

    // Explosion State
    bool m_isExploding{false};
    float m_explosionTimer{0.0f};
    const float m_maxExplosionTime{0.4f};

    // Animation State
    float m_timeAlive{0.0f};
};
