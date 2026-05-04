#include "LobbyState.hpp"
#include "GameState.hpp"
#include "ClientEngine.hpp"
#include <SFML/Network/Dns.hpp>
#include <imgui.h>
#include <iostream>

LobbyState::LobbyState(ClientEngine& engine) : State(engine) {}

void LobbyState::handlePacket(PacketType type, sf::Packet& packet){
    if(type == PacketType::JoinAccept){
        std::uint32_t myId;
        if(packet >> myId){
            std::cout << "[CLIENT] Connected to the server! Joining game as ID: " << myId << "\n";
            m_engine.changeState(std::make_unique<GameState>(m_engine, myId, m_selectedClass));
        }
    }
}

void LobbyState::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH / 2.0f - 200.0f, Config::WINDOW_HEIGHT / 2.0f - 150.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Once);

    ImGui::Begin("Swarm Invasion - M E N U", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::Text("Server IP Address:");
    ImGui::InputText("##ip", m_ipBuffer, sizeof(m_ipBuffer));

    ImGui::Separator();
    ImGui::Text("Choose your class:");
    int classChoice = static_cast<int>(m_selectedClass);
    ImGui::RadioButton("Soldier (Rifle)", &classChoice, 0);
    ImGui::RadioButton("Scout (Laser)", &classChoice, 1);
    ImGui::RadioButton("Tank (Rocket)", &classChoice, 2);
    ImGui::RadioButton("Medic (Rifle)", &classChoice, 3);
    m_selectedClass = static_cast<PlayerClass>(classChoice);
    ImGui::Separator();

    if(m_isConnecting) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Connecting to server...");
    } else {
        if(ImGui::Button("JOIN GAME", ImVec2(-1, 50))){
            m_isConnecting = true;

            auto resolvedIps = sf::Dns::resolve(m_ipBuffer);
            if(resolvedIps.has_value() && !resolvedIps->empty()){
                m_engine.getServerAddress() = resolvedIps->front();

                sf::Packet joinPacket;
                joinPacket << PacketType::JoinRequest << m_selectedClass;
                (void)m_engine.getSocket().send(joinPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
            } else {
                std::cerr << "Invalid IP Address!\n";
                m_isConnecting = false;
            }
        }
    }

    if(ImGui::Button("Quit Game", ImVec2(-1, 30))){
        m_engine.quit();
    }
    ImGui::End();
}