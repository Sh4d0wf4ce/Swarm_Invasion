#include "ClientEngine.hpp"
#include "NetworkProtocol.hpp"
#include <iostream>


ClientEngine::ClientEngine(): m_isRunning(true){
    m_window.create(sf::VideoMode({1280, 720}), "Swarm Invasion - Client", sf::Style::Default);
    m_window.setFramerateLimit(60);

    if(!ImGui::SFML::Init(m_window)){
        std::cerr<<"Error: Failed to initialize ImGui\n";
    }

    m_socket.setBlocking(false);
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
    }
}

void ClientEngine::processNetwork(){
    sf::Packet packet;
    std::optional<sf::IpAddress> sender;
    unsigned short port;

    while(m_socket.receive(packet, sender, port) == sf::Socket::Status::Done){
        PacketType type;
        if(packet >> type){
            if(type == PacketType::Pong){
                std::string message;
                packet >> message;
                std::cout<< "[CLIENT] Recieved PONG from server: " << message << std::endl;
            }
        }
    }
}

void ClientEngine::update(sf::Time deltaTime){
    ImGui::SFML::Update(m_window, deltaTime);

}

void ClientEngine::render(){
    m_window.clear(sf::Color(30, 30, 30));

    renderUI();

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void ClientEngine::renderUI(){
    ImGui::Begin("Swarm Inasion - Debug Panel");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if(ImGui::Button("Send PING to server!")){
        sf::Packet packet;
        std::optional<sf::IpAddress> serverAddress = sf::IpAddress::resolve("127.0.0.1");

        if(serverAddress){
            packet << PacketType::Ping << "Hello Server, are you there?";
            if(m_socket.send(packet, serverAddress.value(), 54000) != sf::Socket::Status::Done){
                std::cout << "[CLIENT] Error sending PING...\n";
            }else{
                std::cout << "[CLIENT] Sent PING...\n";
            }
        }else{
            std::cerr<<"[CLIENT] IP resolve error!\n";
        }
    }

    if(ImGui::Button("Close game")){
        m_isRunning = false;
    }

    ImGui::End();
}