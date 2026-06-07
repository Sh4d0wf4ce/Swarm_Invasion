#include "Soldier.hpp"
#include "../core/ClientEngine.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include "AbilityRegistry.hpp"
#include <imgui.h>

Soldier::Soldier(std::uint32_t id, const sf::Vector2f& startPos) : Player(id, startPos, PlayerClass::Soldier){
    const auto& a = AbilityRegistry::soldier();
    m_maxCooldownRMB = a.Rocket.cooldown;
    m_maxCooldownE = a.HealField.cooldown;
}


void Soldier::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    const auto& sprint = AbilityRegistry::soldier().Sprint;
    const float sprintMult = AbilityRegistry::param(sprint, "speedMultiplier", 1.6f);
    const float staminaDrain = AbilityRegistry::param(sprint, "staminaDrain", 30.f);
    const float staminaRegen = AbilityRegistry::param(sprint, "staminaRegen", 20.f);

    // --- Ult timer ---
    if(m_isUltActive){
        m_ultTimer -= deltaTime.asSeconds();
        if(m_ultTimer <= 0.0f){
            m_isUltActive = false;
            m_fireRateMultiplier = 1.0f;
        }
    }
    m_isSprinting = false;

    // --- Sprint input ---
    bool isMoving = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&!m_isExhausted && isMoving){
        m_isSprinting = true;
    }

    // --- Stamina drain / regen ---
    if(m_isSprinting){
        m_speedMultiplier = sprintMult;
        m_stamina -= staminaDrain * deltaTime.asSeconds();
        if(m_stamina <= 0.0f){
            m_stamina = 0.0f;
            m_isExhausted = true;
        }
    }else{
        m_speedMultiplier = 1.0f;
        if(m_stamina < m_maxStamina){
            m_stamina += staminaRegen * deltaTime.asSeconds();
            if(m_stamina >= m_maxStamina) {
                m_stamina = m_maxStamina;
                m_isExhausted = false;
            }
        }
    }

    Player::update(deltaTime, map);
}

// ==========================================
// Ability inputs
// ==========================================

void Soldier::onQ(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if (m_ultCharge >= m_maxUltCharge) {
        m_isUltActive = true;
        m_ultTimer = AbilityRegistry::soldier().Ult.duration;
        m_ultCharge = 0.0f;
        m_fireRateMultiplier = AbilityRegistry::param(AbilityRegistry::soldier().Ult, "fireRateMultiplier", 0.5f);
        m_ammo = m_maxAmmo;
        m_isReloading = false;
    }
}

void Soldier::onE(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if (m_cooldownE <= 0.0f) {
        m_cooldownE = m_maxCooldownE;
        if (engine.getServerAddress()) {
            sf::Packet skillPacket;
            skillPacket << PacketType::AbilityUsed << m_id << AbilityType::SoldierHealField << m_position;
            (void)engine.getSocket().send(skillPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

void Soldier::onRMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr){
    if(m_cooldownRMB <= 0.0f){
        m_cooldownRMB = m_maxCooldownRMB;
        WeaponType weapon = WeaponType::Rocket;
        projMgr.spawnProjectile(m_id, m_position, mouseWorldPos, weapon, Faction::Players);
        if(engine.getServerAddress()){
            sf::Packet shootPacket;
            shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << mouseWorldPos;
            (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }
}

void Soldier::onLMB(const sf::Vector2f& mouseWorldPos, ClientEngine& engine, ProjectileManager& projMgr, const std::map<std::uint32_t, std::unique_ptr<Enemy>>& enemies){
    if(m_ammo > 0 && !m_isReloading &&  m_cooldownLMB <= 0.0f){
        m_cooldownLMB = m_fireRate * m_fireRateMultiplier;
        if(!m_isUltActive) m_ammo--;
        sf::Vector2f targetWorldPos = mouseWorldPos;
        if (m_isUltActive) {
            const float autoAimRange = AbilityRegistry::soldier().AutoAim.range;
            float bestDistSq = autoAimRange * autoAimRange;
            for (const auto& [id, enemy] : enemies) {
                (void)id;
                float distSq = (enemy->getPosition() - mouseWorldPos).lengthSquared();
                if (distSq < bestDistSq) {
                    bestDistSq = distSq;
                    targetWorldPos = enemy->getPosition();
                }
            }
        }

        WeaponType weapon = HeroRegistry::getStats(m_class).defaultWeapon;
        projMgr.spawnProjectile(m_id, m_position, targetWorldPos, weapon, Faction::Players);
        if (engine.getServerAddress()) {
            sf::Packet shootPacket;
            shootPacket << PacketType::PlayerShoots << m_id << weapon << m_position << targetWorldPos;
            (void)engine.getSocket().send(shootPacket, engine.getServerAddress().value(), Config::SERVER_PORT);
        }
    }else if(m_ammo == 0 && !m_isReloading){
        reload();
    }
}

// ==========================================
// UI panel
// ==========================================
void Soldier::renderShiftSkill(){
    ImGui::Text("SHIFT");
    float staminaProgress = m_stamina / m_maxStamina;
    if (m_isExhausted) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
    }

    ImGui::ProgressBar(staminaProgress, ImVec2(80.0f, 15.0f), "");
    ImGui::PopStyleColor();
}
