#include "ClientEngine.hpp"
#include "../states/LobbyState.hpp"
#include <iostream>


ClientEngine::ClientEngine(): m_isRunning(true){
    m_window.create(sf::VideoMode({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT}), "Swarm Invasion - Client", sf::Style::Default);
    m_window.setFramerateLimit(60);

    if(!ImGui::SFML::Init(m_window)){
        std::cerr<<"Error: Failed to initialize ImGui\n";
    }

    m_camera.setSize({static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)});

    m_socket.setBlocking(false);
    auto resolvedIps = sf::Dns::resolve("127.0.0.1");
    
    if (resolvedIps.has_value() && !resolvedIps->empty()) {
        m_serverAddress = resolvedIps->front(); 
    } else {
        std::cerr << "[CLIENT ERROR] Failed to resolve IP address!\n";
        m_serverAddress = std::nullopt;
    }

    changeState(std::make_unique<LobbyState>(*this));
}

void ClientEngine::run(){
    while(m_isRunning && m_window.isOpen()){
        sf::Time deltaTime = m_clock.restart();

        processEvent();
        processNetwork();
        update(deltaTime);
        render();
    }

    ImGui::SFML::Shutdown();
}

void ClientEngine::processEvent(){
    while(const std::optional event = m_window.pollEvent()){
        ImGui::SFML::ProcessEvent(m_window, *event);

        if(event->is<sf::Event::Closed>()){
            m_isRunning = false;
            m_window.close();
        }

        if(m_currentState) m_currentState->handleInput(*event);
    }
}

void ClientEngine::processNetwork(){
    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while(m_socket.receive(packet, sender, port) == sf::Socket::Status::Done){
        PacketType type;
        if(packet >> type){
            if(m_currentState) m_currentState->handlePacket(type, packet);
        }
    }
}


void ClientEngine::update(sf::Time deltaTime){
    ImGui::SFML::Update(m_window, deltaTime);

    if(m_currentState) m_currentState->update(deltaTime);
}

void ClientEngine::render(){
    m_window.clear(sf::Color(30, 30, 30));

    if(m_currentState){
        m_currentState->render();
        m_currentState->renderUI();
    }

    ImGui::SFML::Render(m_window);
    m_window.display();
}


void ClientEngine::changeState(std::unique_ptr<State> newState){
    if(m_currentState) m_currentState->onExit();
    m_currentState = std::move(newState);
    if(m_currentState) m_currentState->onEnter();
}