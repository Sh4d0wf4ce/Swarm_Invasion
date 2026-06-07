#pragma once
#include "Player.hpp"
#include <SFML/Graphics/VertexArray.hpp>

/**
 * @brief Heavy shotgun hero with charge dash, repulsor cone, and black hole.
 *
 * Uses limited shotgun ammo with reload, a directional charge on Shift, a
 * frontal repulsor blast on right click, black hole placement on E, and
 * rapid-fire recoil shooting during the ultimate.
 */
class Juggernaut : public Player{
public:
    Juggernaut(std::uint32_t id, const sf::Vector2f& startPos);
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;


    // Ability Input Hooks
    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;
    bool isAutoFiring() const override { return m_isUltActive; }


    // Combat & Remote Sync
    std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) override;
    void updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void playRemoteAbility(AbilityType ability, const sf::Vector2f& data) override;

private:
    // Charge Dash Mechanics
    bool m_isCharging{false};
    float m_chargeDistanceRemaining{0.0f};
    float m_maxChargeDistance{350.0f};
    float m_chargeSpeedMultiplier{3.5f};
    float m_shiftCooldown{5.0f};
    float m_dashCollisionBonus{15.0f};
    sf::Vector2f m_chargeDirection{0.0f, 0.0f};
    std::vector<std::uint32_t> m_dashedEnemies;


    // Repulsor Mechanics
    float m_repulsorRange{250.0f};
    float m_repulsorAngleSpan{70.0f * (M_PI / 180.0f)};
    float m_repulsorVfxDuration{0.2f};
    bool m_fireRepulsor{false};
    sf::Vector2f m_repulsorAimDir{1.0f, 0.0f};
    float m_repulsorVfxTimer{0.0f};
    sf::VertexArray m_repulsorVfx{sf::PrimitiveType::TriangleFan};


    // Ultimate & Recoil
    float m_blackHoleMaxRange{200.0f};
    float m_ultDuration{5.0f};
    float m_recoilForce{300.0f};
    sf::Vector2f m_recoilVelocity{0.0f, 0.0f};
};
