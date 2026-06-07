#pragma once
#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "AbilityRegistry.hpp"
#include "Config.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

/**
 * @brief Vanguard decoy entity that mimics the hero and explodes on destruction.
 *
 * Pulses its outline while alive, then plays an expanding cyan explosion VFX
 * when triggered. Uses Vanguard hero visuals and player faction alignment.
 */
class Decoy : public Entity {
public:
    /**
     * @brief Constructs a decoy with Vanguard appearance and configurable HP.
     * @param id Unique network entity identifier.
     * @param startPos Initial world position.
     * @param maxHp Maximum hit points; uses registry default when negative.
     */
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


    /**
     * @brief Updates pulse animation and explosion expansion VFX.
     * @param deltaTime Elapsed time since the last frame.
     * @param map Tile map reference (unused).
     */
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override {
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

    /**
     * @brief Draws the decoy shape or its explosion effect.
     * @param target Render target to draw into.
     */
    void render(sf::RenderTarget& target) override {
        if (!m_isExploding) {
            target.draw(m_shape);
            drawHealthBar(target, 30.0f);
        } else {
            target.draw(m_explosionShape);
        }
    }

    // Explosion Control
    /**
     * @brief Switches the decoy into its explosion animation state.
     */
    void triggerExplosion() { m_isExploding = true; }

    /**
     * @brief Reports whether the decoy is currently exploding.
     * @return True while the explosion VFX is active.
     */
    bool isExploding() const { return m_isExploding; }

    /**
     * @brief Reports whether the explosion animation has completed.
     * @return True when exploding and the timer has elapsed.
     */
    bool isFinished() const { return m_isExploding && (m_explosionTimer >= m_maxExplosionTime); }

    /**
     * @brief Returns the player faction for combat filtering.
     * @return Faction::Players.
     */
    Faction getFaction() const override { return Faction::Players; }

    /**
     * @brief Returns the Vanguard hero collision radius.
     * @return Radius from HeroRegistry for PlayerClass::Vanguard.
     */
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
