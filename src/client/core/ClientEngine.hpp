#pragma once

#include "../entities/Player.hpp"
#include "../entities/Enemy.hpp"
#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "MapRenderer.hpp"
#include "../projectiles/ProjectileManager.hpp"
#include "Config.hpp"
#include "../states/State.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <algorithm>
#include <memory>
#include <map>

class ClientEngine{
public:
    ClientEngine();
    void run();

    void changeState(std::unique_ptr<State> newState);

    sf::RenderWindow& getWindow() { return m_window; }
    sf::UdpSocket& getSocket() { return m_socket; }
    std::optional<sf::IpAddress>& getServerAddress() { return m_serverAddress; }
    void quit() { m_isRunning = false; }

private:
    void processEvent();
    void processNetwork();
    void handleWorldState(sf::Packet& packet);
    void update(sf::Time deltaTime);
    void render();
    void renderUI();

    sf::UdpSocket m_socket;
    sf::RenderWindow m_window;
    sf::View m_camera;
    sf::Clock m_clock;
    bool m_isRunning;

    std::optional<sf::IpAddress> m_serverAddress;

    std::unique_ptr<State> m_currentState;
};