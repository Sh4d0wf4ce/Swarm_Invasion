#pragma once

#include "Player.hpp"

class Juggernaut : public Player{
public:
    Juggernaut(std::uint32_t id, const sf::Vector2f& startPos);

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;

    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;

    std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) override;
private:
    bool m_isCharging{false};
    float m_chargeDistanceRemaining{0.0f};
    const float m_maxChargeDistance{350.0f};
    sf::Vector2f m_chargeDirection{0.0f, 0.0f};

    bool m_fireRepulsor{false};
    sf::Vector2f m_repulsorAimDir{1.0f, 0.0f};
    float m_repulsorVfxTimer{0.0f};
    sf::VertexArray m_repulsorVfx{sf::PrimitiveType::TriangleFan};

    std::vector<std::uint32_t> m_dashedEnemies;
};