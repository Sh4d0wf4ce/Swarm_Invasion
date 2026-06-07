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

/**
 * @brief Central client application engine.
 *
 * Owns the render window, UDP socket, and active game state. Runs the main
 * loop that polls input, receives network packets, updates the current state,
 * and draws the frame.
 */
class ClientEngine{
public:
    ClientEngine();
    void run();

    void changeState(std::unique_ptr<State> newState);

    // Engine Accessors
    /** @brief Returns the SFML render window used for drawing and input. */
    sf::RenderWindow& getWindow() { return m_window; }

    /** @brief Returns the non-blocking UDP socket used for server communication. */
    sf::UdpSocket& getSocket() { return m_socket; }

    /** @brief Returns the resolved server IP address, if available. */
    std::optional<sf::IpAddress>& getServerAddress() { return m_serverAddress; }

    /** @brief Requests graceful shutdown of the main loop on the next iteration. */
    void quit() { m_isRunning = false; }
private:
    // Main Loop
    void processEvent();
    void processNetwork();
    void handleWorldState(sf::Packet& packet);
    void update(sf::Time deltaTime);
    void render();
    void renderUI();

    // Engine State
    sf::UdpSocket m_socket;
    sf::RenderWindow m_window;
    sf::View m_camera;
    sf::Clock m_clock;
    bool m_isRunning;
    std::optional<sf::IpAddress> m_serverAddress;
    std::unique_ptr<State> m_currentState;
};
