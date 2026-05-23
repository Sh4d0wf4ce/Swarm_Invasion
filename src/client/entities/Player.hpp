#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "WeaponRegistry.hpp"

#include <algorithm>

class ClientEngine;
class ProjectileManager;
class Enemy;

class Player : public Entity{
public:
    static std::unique_ptr<Player> create(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass)    ;

    Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass);
    virtual ~Player() = default;

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;
    void render(sf::RenderTarget& target) override;
    virtual void renderUI();

    void setFocused(bool focuesd);
    PlayerClass getClass() const { return m_class; }
    Faction getFaction()  const override { return Faction::Players; }
    float getRadius() const override { return HeroRegistry::getStats(m_class).radius; }


    virtual void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {}
    virtual void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {}
    virtual void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {}
    virtual void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) {}
    virtual void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) {}

    virtual void reload();
    virtual void addUltCharge(float amount);
    bool isUltActive() const { return m_isUltActive; }
    

    int getAmmo() const { return m_ammo; }
    bool isReloading() const { return m_isReloading; }

protected:
    bool checkCollision(const sf::Vector2f& pos, const std::shared_ptr<MapGenerator>& map);

    virtual void renderLeftPanel();
    virtual void renderRightPanel();

    virtual void renderShiftSkill();
    virtual void renderESkill();
    virtual void renderQSkill();
    virtual void renderRMBSkill();

    sf::CircleShape m_shape;
    float m_speed;
    bool m_isFocused;
    PlayerClass m_class;

    sf::Vector2f m_lastMoveDirection{1.0f, 0.0f};

    float m_cooldownShift{0.0f};
    float m_cooldownE{0.0f};
    float m_cooldownRMB{0.0f};
    float m_cooldownLMB{0.0f};
    
    float m_maxCooldownShift{1.0f};
    float m_maxCooldownE{1.0f};
    float m_maxCooldownRMB{1.0f};
    float m_fireRate{0.15f};

    int m_ammo{30};
    int m_maxAmmo{30};
    bool m_isReloading{false};
    float m_reloadTimer{0.0f};
    float m_reloadTime{1.5f};

    float m_ultCharge{0.0f};
    float m_maxUltCharge{500.0f};
    float m_ultTimer{0.0f};
    bool m_isUltActive{false};

    float m_fireRateMultiplier{1.0f};
    float m_speedMultiplier{1.0f};
};


class ScoutPlayer : public Player {
public:
    using Player::Player;
    
    void onShift(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
};