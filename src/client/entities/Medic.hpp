#pragma once
#include "Player.hpp"

class Medic : public Player {
public:
    Medic(std::uint32_t id, const sf::Vector2f& startPos);

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;

    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;

    void setDroneState(bool active, float lifetime);
    bool hasActiveDrone() const { return m_droneActive; }

    void renderQSkill() override;

    std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) override;

    
private:
    enum class TeleportPhase { None, FadeOut, FadeIn };

    bool computeTeleportTarget(const sf::Vector2f& mouseWorldPos, const std::shared_ptr<MapGenerator>& map, sf::Vector2f& outTarget);
    void tryStartTeleport(const std::shared_ptr<MapGenerator>& map);

    TeleportPhase m_teleportPhase{TeleportPhase::None};
    float m_teleportAnimTime{0.0f};
    sf::Vector2f m_teleportTarget;
    sf::Color m_baseFillColor;
    sf::Color m_baseOutlineColor;

    bool m_shiftRequested{false};
    sf::Vector2f m_shiftMousePos;
    ClientEngine* m_shiftEngine{nullptr};

    bool m_droneActive{false};
    float m_droneLifetime{0.0f};
};
