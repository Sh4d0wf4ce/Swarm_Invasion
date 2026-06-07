#pragma once
#include "Player.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include <vector>

/**
 * @brief Melee-focused hero with katana slashes, dash charges, and shuriken burst.
 *
 * Uses katana sweeps on left click, shuriken bursts on right click, a
 * charge-based dash on Shift, a decoy stealth ability on E, and a speed
 * boost ultimate on Q. Supports local hit detection and remote VFX replay.
 */
class Vanguard : public Player {
public:
    Vanguard(std::uint32_t id, const sf::Vector2f& startPos);
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;
    void reload() override {} 
    void renderRightPanel() override;

    // Ability Input Hooks
    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;


    // Combat & Remote Sync
    std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) override;
    void updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void playRemoteAbility(AbilityType ability, const sf::Vector2f& data) override;

private:
    // HUD Overrides
    void renderShiftSkill() override;
    void renderESkill() override;


    // Katana Geometry Helpers
    struct SlashTrailFrame {
        float angle{0.0f};
        float age{0.0f};
    };

    float getBladeAngleAt(float progress) const;
    sf::Vector2f getBladePoint(float angle, float radiusProgress) const;
    float normalizeAngle(float angle) const;
    bool isAngleBetween(float target, float angle1, float angle2) const;


    // Dash Mechanics
    struct DashTrail{
        sf::Vector2f start;
        sf::Vector2f end;
        float age{0.0f};
    };
    bool m_isDashing{false};
    sf::Vector2f m_dashDir{0.0f, 0.0f};
    float m_dashDistanceRemaining{0.0f};
    sf::Vector2f m_dashStartPos;
    std::unordered_set<std::uint32_t> m_hitDuringDash;
    std::vector<DashTrail> m_dashTrails;
    int m_dashCharges{0};
    int m_maxDashCharges{0};
    float m_dashRechargeTimer{0.0f};
    float m_dashRechargeTime{2.5f};
    float m_dashDistance{220.0f};
    float m_dashSpeed{2500.0f};
    float m_dashHitRadiusBonus{25.0f};
    float m_dashTrailFade{0.35f};


    // Shuriken Burst
    int m_shurikensToFire{0};
    float m_shurikenBurstTimer{0.0f};
    sf::Vector2f m_shurikenAimDir{1.0f, 0.0f};
    ClientEngine* m_engineRef{nullptr};
    ProjectileManager* m_projMgrRef{nullptr};


    // Katana Sweep Mechanics
    bool m_attackActive{false};
    bool m_swingRight{true};
    sf::Vector2f m_aimDir{1.0f, 0.0f};
    float m_attackTime{0.0f};
    float m_bladeAngle{0.0f};
    float m_prevBladeAngle{0.0f};
    std::unordered_set<std::uint32_t> m_hitThisSwing;
    std::vector<SlashTrailFrame> m_trail;
    float m_swingDuration{0.25f};
    float m_halfArcRad{55.0f * (M_PI / 180.0f)};
    float m_innerRadius{20.0f};
    float m_outerRadius{95.0f};
    float m_katanaWidth{4.0f};
    const int   m_radialSegments{10};


    // Decoy & Stealth Tuning
    float m_stealthDuration{4.0f};


    // Rendering Helpers
    void updateTrail(float dt);
    void drawDashTrails(sf::RenderTarget& target) const;
    void drawTrail(sf::RenderTarget& target) const;
    void drawKatana(sf::RenderTarget& target) const;
};
