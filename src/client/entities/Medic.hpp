#pragma once
#include "Player.hpp"

/**
 * @brief Support hero with teleport, healing orb, barrier wall, and drone ultimate.
 *
 * Fires healing needles on left click, launches directional orbs on right
 * click, teleports on Shift with a fade animation, places arc barriers on E,
 * and commands a healing drone ultimate on Q.
 */
class Medic : public Player {
public:
    Medic(std::uint32_t id, const sf::Vector2f& startPos);
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;


    // Ability Input Hooks
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;


    // Drone Ultimate State
    void setDroneState(bool active, float lifetime);
    bool hasActiveDrone() const { return m_droneActive; }
    bool isTeleportAnimating() const { return m_teleportPhase != TeleportPhase::None; }


    // Remote Sync & Combat
    void updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void playRemoteAbility(AbilityType ability, const sf::Vector2f& data) override;
    void renderQSkill() override;
    std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) override;

private:
    // Teleport System
    enum class TeleportPhase { None, FadeOut, FadeIn };
    
    bool computeTeleportTarget(const sf::Vector2f& mouseWorldPos, const std::shared_ptr<MapGenerator>& map, sf::Vector2f& outTarget);
    void tryStartTeleport(const std::shared_ptr<MapGenerator>& map);
    void advanceTeleportAnimation(float dt);

    TeleportPhase m_teleportPhase{TeleportPhase::None};
    float m_teleportAnimTime{0.0f};
    sf::Vector2f m_teleportTarget;
    sf::Color m_baseFillColor;
    sf::Color m_baseOutlineColor;
    bool m_shiftRequested{false};
    sf::Vector2f m_shiftMousePos;
    ClientEngine* m_shiftEngine{nullptr};


    // Drone Runtime State
    bool m_droneActive{false};
    float m_droneLifetime{0.0f};
};
