#pragma once

#include "State.hpp"
#include "NetworkProtocol.hpp"
#include "HeroRegistry.hpp"
#include "Config.hpp"

#include <SFML/System/Clock.hpp>
#include <string>

class LobbyState : public State {
public:
    LobbyState(ClientEngine& engine);
    ~LobbyState() override = default;

    void handlePacket(PacketType type, sf::Packet& packet) override;
    void handleInput(const sf::Event& event) override {}
    void update(sf::Time deltaTime) override;
    void render() override {}
    void renderUI() override;

private:
    void trySendJoinRequest();
    void cancelConnecting();

    PlayerClass m_selectedClass = PlayerClass::Soldier;
    char m_ipBuffer[64] = "127.0.0.1";
    bool m_isConnecting = false;
    bool m_connectFailed = false;
    std::string m_connectErrorMessage;
    sf::Clock m_connectTimer;
    sf::Clock m_retryTimer;
};
