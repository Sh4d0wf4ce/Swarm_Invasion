#pragma once

#include "Entity.hpp"
#include "HeroRegistry.hpp"
#include "WeaponRegistry.hpp"

#include <algorithm>

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


    virtual void onShift(const sf::Vector2f& mouseWorldPos) {}
    virtual void onQ(const sf::Vector2f& mouseWorldPos) {}
    virtual void onE(const sf::Vector2f& mouseWorldPos) {}

    virtual bool canUsePrimary() const;
    virtual void usePrimary();
    virtual WeaponType getPrimaryWeapon() const { return HeroRegistry::getStats(m_class).defaultWeapon; }

    virtual bool canUseSecondary() const { return false; }
    virtual void useSecondary() {}
    virtual WeaponType getSecondaryWeapon() const { return WeaponType::None; }

    virtual bool canUseSkillE() const { return false; }
    virtual void useSkillE() {}

    int getAmmo() const { return m_ammo; }
    bool isReloading() const { return m_isReloading; }
    virtual void reload();

    virtual void addUltCharge(float amount);
    virtual bool canUseUltimate() const { return m_ultCharge >= m_maxUltCharge; }
    virtual void useUltimate() {}

    virtual bool hasAutoAim() const { return false; }

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

    sf::Clock m_cooldownShift;
    sf::Clock m_cooldownQ;
    sf::Clock m_cooldownE;

    int m_ammo{30};
    int m_maxAmmo{30};
    bool m_isReloading{false};
    float m_reloadTimer{0.0f};
    float m_reloadTime{1.5f};

    float m_fireCooldown{0.0f};
    float m_fireRate{0.15f};

    float m_ultCharge{0.0f};
    float m_maxUltCharge{500.0f}; //500

    float m_fireRateMultiplier{1.0f};
    float m_speedMultiplier{1.0f};
};


class ScoutPlayer : public Player {
public:
    using Player::Player;
    
    void onShift(const sf::Vector2f& mouseWorldPos) override;
};