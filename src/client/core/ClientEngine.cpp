#include "ClientEngine.hpp"
#include "../states/LobbyState.hpp"
#include <iostream>

// ==========================================
// Construction
// ==========================================

/**
 * @brief Constructs the client engine and enters the lobby state.
 *
 * Creates the window, initializes ImGui-SFML, configures the camera and
 * non-blocking UDP socket, resolves the default server address, and
 * transitions to LobbyState.
 */
ClientEngine::ClientEngine(): m_isRunning(true){
    m_window.create(
        sf::VideoMode({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT}),
        "Swarm Invasion - Client",
        sf::Style::Titlebar | sf::Style::Close
    );
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

// ==========================================
// Main Loop
// ==========================================

/**
 * @brief Runs the main application loop until quit or window close.
 *
 * Each frame polls events, processes incoming UDP packets, updates the
 * active state, and renders the scene. Shuts down ImGui-SFML on exit.
 */
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

// ==========================================
// Event Processing
// ==========================================

/**
 * @brief Polls and dispatches window and ImGui events.
 *
 * Forwards each event to ImGui-SFML and the current state. Closes the
 * application when the window close event is received and restores the
 * configured window size on resize events.
 */
void ClientEngine::processEvent(){
    while(const std::optional event = m_window.pollEvent()){
        ImGui::SFML::ProcessEvent(m_window, *event);
        if(event->is<sf::Event::Closed>()){
            m_isRunning = false;
            m_window.close();
        }

        if(event->is<sf::Event::Resized>()){
            m_window.setSize({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT});
        }
        if(m_currentState) m_currentState->handleInput(*event);
    }
}

// ==========================================
// Network Processing
// ==========================================

/**
 * @brief Receives and dispatches all pending UDP packets from the server.
 *
 * Drains the non-blocking socket and forwards each decoded PacketType to
 * the current state's handlePacket handler.
 */
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

// ==========================================
// Update & Render
// ==========================================

/**
 * @brief Advances ImGui and the active state by one frame.
 * @param deltaTime Elapsed time since the previous frame.
 */
void ClientEngine::update(sf::Time deltaTime){
    ImGui::SFML::Update(m_window, deltaTime);
    if(m_currentState) m_currentState->update(deltaTime);
}

/**
 * @brief Clears the window, renders the active state, and presents the frame.
 *
 * Invokes the state's world render and UI render paths, then draws ImGui
 * overlays and swaps the framebuffer.
 */
void ClientEngine::render(){
    m_window.clear(sf::Color(30, 30, 30));
    if(m_currentState){
        m_currentState->render();
        m_currentState->renderUI();
    }

    ImGui::SFML::Render(m_window);
    m_window.display();
}

// ==========================================
// State Management
// ==========================================

/**
 * @brief Replaces the active state with a new one.
 * @param newState Ownership of the next state; may be null to clear the current state.
 */
void ClientEngine::changeState(std::unique_ptr<State> newState){
    if(m_currentState) m_currentState->onExit();
    m_currentState = std::move(newState);
    if(m_currentState) m_currentState->onEnter();
}
