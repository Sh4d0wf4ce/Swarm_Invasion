#pragma once



/**
 * @brief Global compile-time constants for networking, rendering, map layout, and gameplay tuning.
 *
 * Values in this namespace are shared by client and server and define the baseline
 * configuration for window size, map dimensions, experience pickup behavior, and UI timing.
 */

namespace Config{



    // ==========================================
    // Network
    // ==========================================
    constexpr unsigned short SERVER_PORT = 54000;
    constexpr float NETWORK_TIMEOUT_SECONDS = 3.0f;



    // ==========================================
    // Window & Camera
    // ==========================================
    constexpr unsigned int WINDOW_WIDTH = 1280;
    constexpr unsigned int WINDOW_HEIGHT = 720;
    constexpr unsigned int FPS_LIMIT = 60;


    // ==========================================
    // Map
    // ==========================================
    constexpr float TILE_SIZE = 64.0f;
    constexpr int MAP_WIDTH_TILES = 250;
    constexpr int MAP_HEIGHT_TILES = 250;
    constexpr int MAP_FILL_PERCENT = 45;


    // ==========================================
    // Exp System
    // ==========================================
    constexpr float MAGNET_RADIUS = 150.0f;
    constexpr float PICKUP_RADIUS = 30.0f;
    constexpr float CRYSTAL_SPEED = 400.0f;


    // ==========================================
    // Level Up Window
    // ==========================================
    constexpr float LEVEL_UP_TIMEOUT = 15.0f;
    constexpr float LEVEL_UP_REVEAL_DELAY = 1.0f;


    // ==========================================
    // Lobby
    // ==========================================
    constexpr float LOBBY_CONNECT_TIMEOUT = 8.0f;
    constexpr float LOBBY_JOIN_RETRY_INTERVAL = 1.0f;


    // ==========================================
    // HUD
    // ==========================================
    constexpr float EXP_BAR_WIDTH = 420.0f;
    constexpr float EXP_BAR_HEIGHT = 28.0f;

}