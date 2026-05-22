#include "Soldier.hpp"
#include <imgui.h>

void Soldier::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
    if(m_isUltActive){
        m_ultTimer -= deltaTime.asSeconds();
        if(m_ultTimer <= 0.0f){
            m_isUltActive = false;
            m_fireRateMultiplier = 1.0f;
        }
    }

    if(m_rocketCooldown > 0.0f) m_rocketCooldown -= deltaTime.asSeconds();
    if(m_healCooldown > 0.0f) m_healCooldown -= deltaTime.asSeconds();

    m_isSprinting = false;

    bool isMoving = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || 
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) &&!m_isExhausted && isMoving){
        m_isSprinting = true;
    }

    float currentSpeed = m_speed;

    if(m_isSprinting){
        m_speedMultiplier = 1.6f;
        m_stamina -= 30.0f * deltaTime.asSeconds();

        if(m_stamina <= 0.0f){
            m_stamina = 0.0f;
            m_isExhausted = true;
        }
    }else{
        m_speedMultiplier = 1.0f;

        if(m_stamina < m_maxStamina){
            m_stamina += 20.0f * deltaTime.asSeconds();
            if(m_stamina >= m_maxStamina) {
                m_stamina = m_maxStamina;
                m_isExhausted = false;
            }
        }
    }

    Player::update(deltaTime, map);
}

bool Soldier::canUseSecondary() const{
    return m_rocketCooldown <= 0.0f;
}

void Soldier::useSecondary(){
    m_rocketCooldown = m_rocketCooldownMax;
}

bool Soldier::canUseSkillE() const {
    return m_healCooldown <= 0.0f;
}

void Soldier::useSkillE() {
    m_healCooldown = m_healCooldownMax;
}

void Soldier::useUltimate() {
    m_isUltActive = true;
    m_ultTimer = 10.0f;
    m_ultCharge = 0.0f;
    m_fireRateMultiplier = 0.5f;

    m_ammo = m_maxAmmo;
    m_isReloading = false;
}

WeaponType Soldier::getSecondaryWeapon() const{
    return WeaponType::Rocket;
}

void Soldier::onShift(const sf::Vector2f& mouseWorldPos){

}

void Soldier::onQ(const sf::Vector2f& mouseWorldPos){

}

void Soldier::onE(const sf::Vector2f& mouseWorldPos){

}

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


void Soldier::renderRMBSkill(){
    ImGui::Text("RMB");

    if(m_rocketCooldown <= 0.0f){
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    }else{
        float progress = 1.0f - (m_rocketCooldown / m_rocketCooldownMax);
        char cdText[16];
        sprintf(cdText, "%.1fs", m_rocketCooldown);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

void Soldier::renderESkill() {
    ImGui::Text("E");
    
    if (m_healCooldown <= 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.8f, 0.2f, 1.0f)); // Zielony
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
        ImGui::PopStyleColor();
    } else {
        float progress = 1.0f - (m_healCooldown / m_healCooldownMax);
        char cdText[16];
        sprintf(cdText, "%.1fs", m_healCooldown);
        ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), cdText);
    }
}

void Soldier::renderQSkill() {
    ImGui::Text("Q (ULTIMATE)");
    
    if (m_isUltActive) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Agresywny czerwony
        char timeText[16];
        sprintf(timeText, "ACTIVE: %.1fs", m_ultTimer);
        ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), timeText);
        ImGui::PopStyleColor();
    } else {
        float progress = m_ultCharge / m_maxUltCharge;
        if (progress >= 1.0f) {
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f, 0.1f, 0.8f, 1.0f)); // Fioletowy - gotowe
            ImGui::ProgressBar(1.0f, ImVec2(80.0f, 15.0f), "READY");
            ImGui::PopStyleColor();
        } else {
            char percText[16];
            sprintf(percText, "%.0f%%", progress * 100.0f);
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.4f, 0.4f, 1.0f)); // Szary - ładuje
            ImGui::ProgressBar(progress, ImVec2(80.0f, 15.0f), percText);
            ImGui::PopStyleColor();
        }
    }
}