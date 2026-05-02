#pragma once

namespace Config{
    // --- NETWORK ---
    constexpr unsigned short SERVER_PORT = 54000;
    constexpr float NETWORK_TIMEOUT_SECONDS = 3.0f;

    // --- WINDOW & CAMERA ---
    constexpr unsigned int WINDOW_WIDTH = 1280;
    constexpr unsigned int WINDOW_HEIGHT = 720;
    constexpr unsigned int FPS_LIMIT = 60;

    // --- MAP ---
    constexpr float TILE_SIZE = 32.0f;
    constexpr int MAP_WIDTH_TILES = 100;
    constexpr int MAP_HEIGHT_TILES = 100;
    constexpr int MAP_FILL_PERCENT = 45;

    // --- ENEMY ---
    constexpr float ENEMY_SPAWN_RATE = 2.0f;

    // --- PROJECTILES ---
    constexpr float PROJECTILE_SPEED = 800.0f;
    constexpr float PROJECTILE_RADIUS = 5.0f;
    constexpr float PROJECTILE_LIFETIME = 3.0f;
    constexpr float PROJECTILE_DAMAGE = 25.0f;
}