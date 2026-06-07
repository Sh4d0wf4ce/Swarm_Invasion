#include "Player.hpp"
#include "Soldier.hpp"
#include "Juggernaut.hpp"
#include "Vanguard.hpp"
#include "Medic.hpp"
#include "imgui.h"

/**
 * @brief Initializes a player with hero stats, shape, and default state.
 * @param id Unique network entity identifier.
 * @param startPos Initial world position.
 * @param pClass Hero class used to look up registry stats.
 */
Player::Player(std::uint32_t id, const sf::Vector2f& startPos, PlayerClass pClass): Entity(id, startPos), m_isFocused(true), m_class(pClass){
    const auto& stats = HeroRegistry::getStats(pClass);
    m_maxHp = stats.maxHp;
    m_baseMaxHp = stats.maxHp;
    m_hp = stats.maxHp;
    m_speed = stats.speed;
    m_shape.setRadius(stats.radius);
    m_shape.setFillColor(stats.color);
    m_shape.setOrigin({stats.radius, stats.radius});
    m_shape.setPosition(m_position);
}

/**
 * @brief Advances reload and cooldown timers, then applies keyboard movement.
 * @param deltaTime Elapsed time since the last frame.
 * @param map Tile map used for wall collision checks.
 */
void Player::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    // --- Advance reload timer and restore ammo when complete ---
    if(m_isReloading){
        m_reloadTimer -= deltaTime.asSeconds();
        if(m_reloadTimer <= 0.0f){
            m_isReloading = false;
            m_ammo = m_maxAmmo;
        }
    }

    // --- Tick ability cooldowns scaled by upgrade modifiers ---
    const float cdRate = 1.0f / getUpgradeCooldownScale();
    if (m_cooldownShift > 0.0f) m_cooldownShift -= deltaTime.asSeconds() * cdRate;
    if (m_cooldownE > 0.0f) m_cooldownE -= deltaTime.asSeconds() * cdRate;
    if (m_cooldownRMB > 0.0f) m_cooldownRMB -= deltaTime.asSeconds() * cdRate;
    if (m_cooldownLMB > 0.0f) m_cooldownLMB -= deltaTime.asSeconds() * cdRate;

    // --- Read keyboard input and normalize movement direction ---
    sf::Vector2f movement(0.0f, 0.0f);
    if(m_isFocused){
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= 1.0f;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += 1.0f;
    }

    float len = movement.length();
    if(len > 0.0f) movement /= len;
    m_lastMoveDirection = movement;

    // --- Apply movement with per-axis collision against map walls ---
    float actualSpeed = m_speed * m_speedMultiplier * m_upgradeSpeedMultiplier;
    sf::Vector2f velocity = movement * actualSpeed * deltaTime.asSeconds();

    sf::Vector2f nextPosX = m_position + sf::Vector2f(velocity.x, 0.0f);
    if(!checkCollision(nextPosX, map)){
        m_position.x = nextPosX.x;
    }

    sf::Vector2f nextPosY = m_position + sf::Vector2f(0.0f, velocity.y);
    if(!checkCollision(nextPosY, map)){
        m_position.y = nextPosY.y;
    }

    m_shape.setPosition(m_position);
}

/**
 * @brief Draws the player circle and health bar.
 * @param target Render target to draw into.
 */
void Player::render(sf::RenderTarget& target){
    m_shape.setPosition(m_position);
    target.draw(m_shape);
    drawHealthBar(target, 30.0f);
}

/**
 * @brief Sets whether this player accepts local keyboard input.
 * @param focused True when the player should respond to movement keys.
 */
void Player::setFocused(bool focused){
    m_isFocused = focused;
}


/**
 * @brief Tests whether a position overlaps a wall tile.
 * @param pos Candidate world position to test.
 * @param map Tile map used for collision lookup.
 * @return True if any corner of the hit box intersects a wall.
 */
bool Player::checkCollision(const sf::Vector2f& pos, const std::shared_ptr<MapGenerator>& map){
    if(!map) return false;
    float hitBoxOffset = HeroRegistry::getStats(m_class).radius * 0.8f;

    sf::Vector2f points[4] = {
        {pos.x - hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y - hitBoxOffset},
        {pos.x - hitBoxOffset, pos.y + hitBoxOffset},
        {pos.x + hitBoxOffset, pos.y + hitBoxOffset}
    };

    for(const auto& p: points){
        int gridX = static_cast<int>(p.x / Config::TILE_SIZE);
        int gridY = static_cast<int>(p.y / Config::TILE_SIZE);
        if(map->getTile(gridX, gridY) == TileType::Wall)
            return true;
    }

    return false;
}

/**
 * @brief Starts a reload if one is not already in progress.
 */
void Player::reload(){
    if(!m_isReloading){
        m_isReloading = true;
        m_reloadTimer = m_reloadTime;
    }
}

/**
 * @brief Adds ultimate charge, clamped to the maximum while ult is inactive.
 * @param amount Charge points to add.
 */
void Player::addUltCharge(float amount){
    if(m_isUltActive) return;
    m_ultCharge += amount;
    if(m_ultCharge > m_maxUltCharge) m_ultCharge = m_maxUltCharge;
}

/**
 * @brief Factory that constructs the correct hero subclass for a player class.
 * @param id Unique network entity identifier.
 * @param startPos Initial world position.
 * @param pClass Hero class to instantiate.
 * @return Owning pointer to the created player subclass.
 */
std::unique_ptr<Player> Player::create(uint32_t id, const sf::Vector2f &startPos, PlayerClass pClass){
    switch(pClass){
        case PlayerClass::Soldier:
            return std::make_unique<Soldier>(id, startPos);
        case PlayerClass::Medic:
            return std::make_unique<Medic>(id, startPos);
        case PlayerClass::Juggernaut:
            return std::make_unique<Juggernaut>(id, startPos);
        case PlayerClass::Vanguard:
            return std::make_unique<Vanguard>(id, startPos);
        default:
            return std::make_unique<Player>(id, startPos, pClass);
    }
}

/**
 * @brief Renders both HUD panels for the local hero.
 */
void Player::renderUI(){
    renderLeftPanel();
    renderRightPanel();
}

/**
 * @brief Renders the right HUD panel with HP and ammo status.
 */
void Player::renderRightPanel() {
    ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH - 320.0f, Config::WINDOW_HEIGHT - 80.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 100.0f), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("HeroStatsRight", nullptr, flags);
    // HP
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "HP: %.0f / %.0f", m_hp, m_maxHp);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
    ImGui::ProgressBar(m_hp / m_maxHp, ImVec2(-1, 15.0f), "");
    ImGui::PopStyleColor();
    // AMMO
    if (m_isReloading) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "RELOADING... %.1fs", std::max(0.0f, m_reloadTimer));
    } else {
        if (m_ammo <= 5) {
            ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "AMMO: %d / %d", m_ammo, m_maxAmmo);
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "AMMO: %d / %d", m_ammo, m_maxAmmo);
        }
    }

    ImGui::End();
}

/**
 * @brief Renders the left HUD panel with ability cooldown indicators.
 */
void Player::renderLeftPanel() {
    ImGui::SetNextWindowPos(ImVec2(20.0f, Config::WINDOW_HEIGHT - 80.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 100.0f), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::Begin("HeroSkillsLeft", nullptr, flags);
    ImGui::Columns(4, "Skills", false);
    renderShiftSkill(); ImGui::NextColumn();
    renderESkill();     ImGui::NextColumn();
    renderRMBSkill();   ImGui::NextColumn();
    renderQSkill();     ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::End();
}

/**
 * @brief Renders the Shift ability cooldown bar in the left HUD panel.
 */
void Player::renderShiftSkill() {
    ImGui::Text("LSHIFT");
    if (m_cooldownShift <= 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        float progress = 1.0f - (m_cooldownShift / getEffectiveMaxCooldown(m_maxCooldownShift));
        char cdText[16];
        sprintf(cdText, "%.1fs", m_cooldownShift);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

/**
 * @brief Renders the E ability cooldown bar in the left HUD panel.
 */
void Player::renderESkill() {
    ImGui::Text("E");
    if (m_cooldownE <= 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        float progress = 1.0f - (m_cooldownE / getEffectiveMaxCooldown(m_maxCooldownE));
        char cdText[16];
        sprintf(cdText, "%.1fs", m_cooldownE);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

/**
 * @brief Renders the Q ultimate charge or active timer in the left HUD panel.
 */
void Player::renderQSkill() {
    ImGui::Text("Q");
    if (m_isUltActive) {
        char timerText[16];
        sprintf(timerText, "%.1fs", std::max(0.0f, m_ultTimer));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.8f, 0.0f, 1.0f)); 
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), timerText);
        ImGui::PopStyleColor();
    } else {
        float progress = m_ultCharge / m_maxUltCharge;
        if (progress >= 1.0f) {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f, 0.1f, 0.8f, 1.0f)); 
            ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
            ImGui::PopStyleColor();
        } else {
            char percText[16];
            sprintf(percText, "%.0f%%", progress * 100.0f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); 
            ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), percText);
            ImGui::PopStyleColor();
        }
    }
}

/**
 * @brief Renders the right-mouse ability cooldown bar in the left HUD panel.
 */
void Player::renderRMBSkill() {
    ImGui::Text("RMB");
    if (m_cooldownRMB <= 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        float progress = 1.0f - (m_cooldownRMB / getEffectiveMaxCooldown(m_maxCooldownRMB));
        char cdText[16];
        sprintf(cdText, "%.1fs", m_cooldownRMB);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

/**
 * @brief Returns the cooldown duration scale from upgrade multipliers.
 * @return Cooldown multiplier clamped to a minimum of 0.1.
 */
float Player::getUpgradeCooldownScale() const {
    return std::max(0.1f, m_upgradeCooldownMultiplier);
}

/**
 * @brief Scales a base cooldown by the current upgrade multiplier.
 * @param baseCooldown Unmodified cooldown duration in seconds.
 * @return Effective maximum cooldown after upgrade scaling.
 */
float Player::getEffectiveMaxCooldown(float baseCooldown) const {
    return baseCooldown * getUpgradeCooldownScale();
}

/**
 * @brief Applies run upgrade multipliers to HP, speed, damage, and cooldowns.
 * @param hpMult Maximum HP multiplier.
 * @param speedMult Movement speed multiplier.
 * @param damageMult Damage multiplier (stored for subclasses).
 * @param cooldownMult Cooldown duration multiplier.
 */
void Player::setUpgradeMultipliers(float hpMult, float speedMult, float damageMult, float cooldownMult) {
    m_upgradeHpMultiplier = hpMult;
    m_upgradeSpeedMultiplier = speedMult;
    m_upgradeCooldownMultiplier = cooldownMult;
    m_maxHp = m_baseMaxHp * hpMult;
}
