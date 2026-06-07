#pragma once
#include "State.hpp"
#include "NetworkProtocol.hpp"
#include "HeroRegistry.hpp"
#include "Config.hpp"
#include <SFML/System/Clock.hpp>
#include <string>

/**
 * @brief Pre-game lobby state for server connection and class selection.
 *
 * Presents an ImGui menu to enter a server address, pick a hero class, and
 * join a match. Retries join requests until accepted or the connection times out.
 */
class LobbyState : public State {
public:
    /**
     * @brief Constructs the lobby state bound to the client engine.
     * @param engine Client engine that owns this state.
     */
    LobbyState(ClientEngine& engine) : State(engine) {}

    /** @brief Default destructor; no special cleanup required. */
    ~LobbyState() override = default;


    // State Interface
    void handlePacket(PacketType type, sf::Packet& packet) override;

    /** @brief Lobby has no keyboard or mouse gameplay input. */
    void handleInput(const sf::Event& event) override {}

    void update(sf::Time deltaTime) override;

    /** @brief Lobby has no world rendering; UI is drawn in renderUI(). */
    void render() override {}

    void renderUI() override;

private:
    // Connection Helpers
    void trySendJoinRequest();
    void cancelConnecting();


    // Lobby & Connection State
    PlayerClass m_selectedClass = PlayerClass::Soldier;
    char m_ipBuffer[64] = "127.0.0.1";
    bool m_isConnecting = false;
    bool m_connectFailed = false;
    std::string m_connectErrorMessage;
    sf::Clock m_connectTimer;
    sf::Clock m_retryTimer;
};
