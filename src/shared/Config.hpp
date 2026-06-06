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
    constexpr float TILE_SIZE = 64.0f;
    constexpr int MAP_WIDTH_TILES = 250;
    constexpr int MAP_HEIGHT_TILES = 250;
    constexpr int MAP_FILL_PERCENT = 45;

    // --- ENEMY ---
    constexpr float ENEMY_SPAWN_RATE = 2.0f;

    // --- PROJECTILES ---
    constexpr float PROJECTILE_SPEED = 800.0f;
    constexpr float PROJECTILE_RADIUS = 5.0f;
    constexpr float PROJECTILE_LIFETIME = 3.0f;
    constexpr float PROJECTILE_DAMAGE = 25.0f;

    // --- EXP SYSTEM ---
    constexpr float MAGNET_RADIUS = 150.0f;
    constexpr float PICKUP_RADIUS = 30.0f;
    constexpr float CRYSTAL_SPEED = 400.0f;

    // --- LEVEL UP WINDOW ---
    constexpr float LEVEL_UP_TIMEOUT = 15.0f;
    constexpr float LEVEL_UP_REVEAL_DELAY = 1.0f;

    // --- LOBBY ---
    constexpr float LOBBY_CONNECT_TIMEOUT = 8.0f;
    constexpr float LOBBY_JOIN_RETRY_INTERVAL = 1.0f;

    // --- HUD ---
    constexpr float EXP_BAR_WIDTH = 420.0f;
    constexpr float EXP_BAR_HEIGHT = 28.0f;

    // --- VANGUARD DECOY (E) ---
    constexpr float VANGUARD_STEALTH_DURATION = 4.0f;
    constexpr float VANGUARD_DECOY_HP = 150.0f;
    constexpr float VANGUARD_DECOY_EXPLOSION_RADIUS = 120.0f;
    constexpr float VANGUARD_DECOY_EXPLOSION_DAMAGE = 200.0f;

    // --- MEDIC TELEPORT (SHIFT) ---
    constexpr float MEDIC_TELEPORT_RANGE = 600.0f;
    constexpr float MEDIC_TELEPORT_RANGE_TOLERANCE = 10.0f;
    constexpr float MEDIC_TELEPORT_COOLDOWN = 6.0f;
    constexpr float MEDIC_TELEPORT_FADE_TOTAL = 0.4f;
    constexpr float MEDIC_TELEPORT_IFRAMES = 0.5f;

    // --- MEDIC BIO-SPHERE (RMB) ---
    constexpr float MEDIC_ORB_COOLDOWN = 8.0f;
    constexpr float MEDIC_ORB_LIFETIME = 6.0f;
    constexpr float MEDIC_ORB_SPEED = 175.0f;
    constexpr float MEDIC_ORB_RADIUS = 15.0f;
    constexpr float MEDIC_ORB_EFFECT_RADIUS = 120.0f;
    constexpr float MEDIC_ORB_TICK_INTERVAL = 0.2f;
    constexpr float MEDIC_ORB_HEAL_PER_TICK = 8.0f;
    constexpr float MEDIC_ORB_DAMAGE_PER_TICK = 12.0f;

    // --- MEDIC NEEDLE (LMB) ---
    constexpr float MEDIC_NEEDLE_HEAL = 15.0f;
    constexpr float MEDIC_NEEDLE_POISON_DURATION = 3.0f;
    constexpr float MEDIC_NEEDLE_POISON_DPS = 5.0f;

    // --- MEDIC BARRIER (E) ---
    constexpr float MEDIC_BARRIER_COOLDOWN = 12.0f;
    constexpr float MEDIC_BARRIER_LIFETIME = 5.0f;
    constexpr float MEDIC_BARRIER_ARC_RADIUS = 280.0f;
    constexpr float MEDIC_BARRIER_STANDOFF = 80.0f;
    constexpr float MEDIC_BARRIER_SPAN = 0.80f;
    constexpr float MEDIC_BARRIER_WALL_THICKNESS = 18.0f;
    constexpr float MEDIC_BARRIER_KNOCKBACK = 900.0f;

    // --- MEDIC PASSIVE ---
    constexpr float MEDIC_PASSIVE_HEAL_PER_SECOND = 2.0f;

    // --- MEDIC DRONE (Q ULTIMATE) ---
    constexpr float MEDIC_DRONE_LIFETIME = 20.0f;
    constexpr float MEDIC_DRONE_ORBIT_RADIUS = 45.0f;
    constexpr float MEDIC_DRONE_ORBIT_SPEED = 2.5f;
    constexpr float MEDIC_DRONE_PLAYER_DETECT_RADIUS = 50.0f;
    constexpr float MEDIC_DRONE_SYM_HEAL_INTERVAL = 1.0f;
    constexpr float MEDIC_DRONE_SYM_HEAL_AMOUNT = 10.0f;
    constexpr float MEDIC_DRONE_SYM_SHOOT_INTERVAL = 0.8f;
    constexpr float MEDIC_DRONE_SENTRY_SHOOT_INTERVAL = 0.1f;
    constexpr float MEDIC_DRONE_MOVE_SPEED = 280.0f;
    constexpr float MEDIC_DRONE_ATTACK_RANGE = 420.0f;
    constexpr float MEDIC_DRONE_SENTRY_ARRIVE_DIST = 8.0f;
}