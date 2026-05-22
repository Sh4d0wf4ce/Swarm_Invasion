#pragma once

#include "Player.hpp"

class Soldier : public Player{
public:
    using Player::Player;

    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;

    void onShift(const sf::Vector2f& mouseWorldPos) override;
    void onQ(const sf::Vector2f& mouseWorldPos) override;
    void onE(const sf::Vector2f& mouseWorldPos) override;

    void usePrimary() override;

    bool canUseSecondary() const override;
    void useSecondary() override;
    WeaponType getSecondaryWeapon() const override;

    float getStamina() const { return m_stamina; }
    float getMaxStamina() const {return m_maxStamina; }
    bool isExhausted() const { return m_isExhausted; }

    bool canUseSkillE() const override;
    void useSkillE() override;

    void useUltimate() override;
    bool hasAutoAim() const override { return m_isUltActive; }

private:
    void renderShiftSkill() override;
    void renderRMBSkill() override;
    void renderESkill() override;
    void renderQSkill() override;

    float m_stamina{100.0f};
    float m_maxStamina{100.0f};
    bool m_isExhausted{false};
    bool m_isSprinting{false};

    float m_rocketCooldown{0.0f};
    const float m_rocketCooldownMax{6.0f};

    float m_healCooldown{0.0f};
    const float m_healCooldownMax{15.0f};
};