#pragma once

#include "Player.hpp"
#include "Enemy.hpp"
#include "NetworkProtocol.hpp"
#include "MapGenerator.hpp"
#include "MapRenderer.hpp"
#include "ProjectileManager.hpp"
#include "Config.hpp"

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

    std::unique_ptr<Player> m_player;
    sf::Vector2f m_lastSentPosition;

    std::map<std::uint32_t, std::unique_ptr<Player>> m_otherPlayers;
    std::map<std::uint32_t, std::unique_ptr<Enemy>> m_enemies;
    
    std::optional<sf::IpAddress> m_serverAddress;
    sf::Clock m_lastServerMessageTimer;

    std::shared_ptr<MapGenerator> m_map;
    std::unique_ptr<MapRenderer> m_mapRenderer;

    std::unique_ptr<ProjectileManager> m_projectileManager;
};