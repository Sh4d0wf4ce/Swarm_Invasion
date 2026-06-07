#include "LobbyState.hpp"
#include "GameState.hpp"
#include "../core/ClientEngine.hpp"
#include <SFML/Network/Dns.hpp>
#include <imgui.h>
#include <iostream>

/**
 * @brief Handles server responses while waiting in the lobby.
 * @param type Identifies the incoming packet kind.
 * @param packet Deserialized UDP payload from the server.
 *
 * On JoinAccept, transitions to GameState with the assigned player id and
 * the class selected in the lobby UI.
 */
void LobbyState::handlePacket(PacketType type, sf::Packet& packet){
    if(type == PacketType::JoinAccept){
        std::uint32_t myId;
        if(packet >> myId){
            m_isConnecting = false;
            m_connectFailed = false;
            std::cout << "[CLIENT] Connected to the server! Joining game as ID: " << myId << "\n";
            m_engine.changeState(std::make_unique<GameState>(m_engine, myId, m_selectedClass));
        }
    }
}

// ==========================================
// Connection Helpers
// ==========================================

/**
 * @brief Resolves the server address and sends a join request packet.
 *
 * Updates connection failure state when DNS resolution fails; otherwise
 * stores the resolved address on the engine and sends JoinRequest with the
 * selected player class.
 */
void LobbyState::trySendJoinRequest(){
    auto resolvedIps = sf::Dns::resolve(m_ipBuffer);
    if(!resolvedIps.has_value() || resolvedIps->empty()){
        m_isConnecting = false;
        m_connectFailed = true;
        m_connectErrorMessage = "Invalid IP address.";
        return;
    }
    m_engine.getServerAddress() = resolvedIps->front();
    sf::Packet joinPacket;
    joinPacket << PacketType::JoinRequest << m_selectedClass;
    (void)m_engine.getSocket().send(joinPacket, m_engine.getServerAddress().value(), Config::SERVER_PORT);
}

/**
 * @brief Aborts an in-progress connection attempt and clears error display state.
 */
void LobbyState::cancelConnecting(){
    m_isConnecting = false;
    m_connectFailed = false;
    m_connectErrorMessage.clear();
}

/**
 * @brief Retries join requests and enforces the lobby connection timeout.
 * @param deltaTime Elapsed time since the previous frame; unused.
 *
 * While connecting, periodically resends join requests and marks the
 * attempt as failed when the configured timeout is exceeded.
 */
void LobbyState::update(sf::Time deltaTime){
    (void)deltaTime;
    if(!m_isConnecting) return;
    if(m_retryTimer.getElapsedTime().asSeconds() >= Config::LOBBY_JOIN_RETRY_INTERVAL){
        m_retryTimer.restart();
        trySendJoinRequest();
    }

    if(m_connectTimer.getElapsedTime().asSeconds() >= Config::LOBBY_CONNECT_TIMEOUT){
        m_isConnecting = false;
        m_connectFailed = true;
        m_connectErrorMessage = "Could not connect to server. Check the IP and make sure the server is running.";
    }
}

// ==========================================
// UI
// ==========================================

/**
 * @brief Renders the lobby menu for server connection and class selection.
 *
 * Displays server IP input, hero class radio buttons, connect/cancel controls,
 * connection status messages, and a quit button.
 */
void LobbyState::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(Config::WINDOW_WIDTH / 2.0f - 200.0f, Config::WINDOW_HEIGHT / 2.0f - 150.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Once);
    ImGui::Begin("Swarm Invasion - M E N U", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    // --- Server address ---
    ImGui::Text("Server IP Address:");
    ImGui::InputText("##ip", m_ipBuffer, sizeof(m_ipBuffer));
    ImGui::Separator();

    // --- Class selection ---
    ImGui::Text("Choose your class:");
    int classChoice = static_cast<int>(m_selectedClass);
    ImGui::RadioButton("Soldier", &classChoice, 0);
    ImGui::RadioButton("Medic", &classChoice, 1);
    ImGui::RadioButton("Juggernaut", &classChoice, 2);
    ImGui::RadioButton("Vanguard", &classChoice, 3);
    m_selectedClass = static_cast<PlayerClass>(classChoice);
    ImGui::Separator();

    // --- Connect / cancel ---
    if(m_isConnecting) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Connecting to server...");
        ImGui::TextWrapped("Retrying every %.0fs. You can cancel and try again.",
            Config::LOBBY_JOIN_RETRY_INTERVAL);
        if(ImGui::Button("CANCEL", ImVec2(-1, 40))){
            cancelConnecting();
        }
    } else {
        if(m_connectFailed){
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", m_connectErrorMessage.c_str());
            ImGui::Spacing();
        }

        if(ImGui::Button("JOIN GAME", ImVec2(-1, 50))){
            m_connectFailed = false;
            m_connectErrorMessage.clear();
            m_isConnecting = true;
            m_connectTimer.restart();
            m_retryTimer.restart();
            trySendJoinRequest();
        }
    }

    if(ImGui::Button("Quit Game", ImVec2(-1, 30))){
        m_engine.quit();
    }
    ImGui::End();
}
