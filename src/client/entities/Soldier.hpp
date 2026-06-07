#pragma once
#include "Player.hpp"

/**
 * @brief Rifle-wielding hero with sprint stamina and area heal.
 *
 * Fires the default hero weapon on left click, launches rockets on right
 * click, drops heal fields on E, and enters a rapid-fire auto-aim ultimate
 * on Q. Sprint is bound to Shift and drains stamina while moving.
 */
class Soldier : public Player{
public:
    Soldier(std::uint32_t id, const sf::Vector2f& startPos);
    void update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map) override;


    // Ability Input Hooks
    void onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr) override;
    void onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies) override;

private:
    // Sprint & Stamina
    float m_stamina{100.0f};
    float m_maxStamina{100.0f};
    bool m_isExhausted{false};
    bool m_isSprinting{false};

    
    void renderShiftSkill() override;
};
