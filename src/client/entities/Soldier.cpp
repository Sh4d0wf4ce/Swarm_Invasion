#include "Soldier.hpp"
#include <imgui.h>

void Soldier::update(sf::Time deltaTime, const std::shared_ptr<MapGenerator>& map){
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

void Soldier::onShift(const sf::Vector2f& mouseWorldPos){

}

void Soldier::onQ(const sf::Vector2f& mouseWorldPos){

}

void Soldier::onE(const sf::Vector2f& mouseWorldPos){

}

void Soldier::renderUI(){
    ImGui::SetNextWindowPos(ImVec2(20.0f, Config::WINDOW_HEIGHT - 100.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    float staminaProgress = m_stamina / m_maxStamina;
    char staminaOverlay[32];
    sprintf(staminaOverlay, "STAMINA: %.0f%%", staminaProgress * 100.0f);

    if (m_isExhausted) {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
        ImGui::ProgressBar(staminaProgress, ImVec2(250.0f, 25.0f), "EXHAUSTED! WAIT...");
    } else {
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
        ImGui::ProgressBar(staminaProgress, ImVec2(250.0f, 25.0f), staminaOverlay);
    }
    ImGui::PopStyleColor();

    ImGui::End();  
}
