#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "WeaponRegistry.hpp"
#include "NetworkProtocol.hpp"

#include <algorithm>

struct AbilityHitRecord{
    std::uint32_t targetId;
    AbilityType ability;
};

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

    virtual void updateRemoteVisuals(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map);
    virtual void playRemoteAbility(AbilityType ability, const sf::Vector2f& data);

    void setFocused(bool focused);
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
    virtual bool isAutoFiring() const { return false; }
    bool isUltActive() const { return m_isUltActive; }
    

    int getAmmo() const { return m_ammo; }
    bool isReloading() const { return m_isReloading; }

    virtual std::vector<AbilityHitRecord> checkAbilityHits(const std::vector<Entity*>& entities) { return {}; }

    void setStealthTimer(float timer){ m_stealthTimer = timer; }
    float getStealthTimer() const { return m_stealthTimer; }

    void setUpgradeMultipliers(float hpMult, float speedMult, float damageMult, float cooldownMult);
    float getUpgradeCooldownScale() const;
    float getEffectiveMaxCooldown(float baseCooldown) const;

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
    float m_stealthTimer{0.0f};

    float m_baseMaxHp{100.0f};
    float m_upgradeHpMultiplier{1.0f};
    float m_upgradeSpeedMultiplier{1.0f};
    float m_upgradeCooldownMultiplier{1.0f};
};
