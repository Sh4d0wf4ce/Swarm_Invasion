#pragma once

#include "Player.hpp"

class Juggernaut : public Player{
public:
    Juggernaut(std::uint32_t id, const sf::Vector2f& startPos);

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;

    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;

private:

    bool m_isCharging{false};
    float m_chargeTimer{0.0f};
    const float m_chargeDuartion{0.5f};
    sf::Vector2f m_chargeDirection{0.0f, 0.0f};
};