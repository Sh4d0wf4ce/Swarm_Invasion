#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

class ClientEngine{
public:
    ClientEngine();

    void run();

private:
    void processEvent();
    void processNetwork();
    void update(sf::Time deltaTime);
    void render();
    void renderUI();

    sf::UdpSocket m_socket;
    sf::RenderWindow m_window;
    sf::Clock m_clock;
    bool m_isRunning;
};