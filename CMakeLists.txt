cmake_minimum_required(VERSION 3.14)
project(SwarmInvasion VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

#SFML
FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sfml)

#ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.5
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()
set(IMGUI_DIR "${imgui_SOURCE_DIR}")

#ImGui-SFML
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
    GIT_TAG v3.0
)
FetchContent_MakeAvailable(imgui-sfml)

#JSON
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)



add_library(SharedStatic STATIC
    src/shared/MapGenerator.cpp
)
target_include_directories(SharedStatic PUBLIC src/shared)
target_link_libraries(SharedStatic PUBLIC nlohmann_json::nlohmann_json SFML::System)

add_executable(SwarmServer
    src/server/core/main.cpp
    src/server/core/ServerEngine.cpp
    src/server/core/AIDirector.cpp
    src/server/enemies/ServerEnemy.cpp
    src/server/enemies/ServerCrawler.cpp
    src/server/enemies/ServerSpitter.cpp
    src/server/enemies/ServerBruiser.cpp
    src/server/enemies/ServerKamikaze.cpp
)
target_link_libraries(SwarmServer PRIVATE SharedStatic SFML::Network SFML::System SFML::Main)

add_executable(SwarmClient
    src/client/core/main.cpp
    src/client/core/ClientEngine.cpp
    src/client/core/MapRenderer.cpp
    src/client/entities/Entity.cpp
    src/client/entities/Player.cpp
    src/client/entities/Enemy.cpp
    src/client/entities/Soldier.cpp
    src/client/states/GameState.cpp
    src/client/states/LobbyState.cpp
    src/client/projectiles/ProjectileManager.cpp
    src/client/projectiles/Projectile.cpp
    src/client/projectiles/LaserProjectile.cpp
    src/client/projectiles/RifleProjectile.cpp
    src/client/projectiles/RocketProjectile.cpp
    src/client/projectiles/AcidProjectile.cpp
)
target_link_libraries(SwarmClient PRIVATE
    SharedStatic
    SFML::Graphics
    SFML::Window
    SFML::Audio
    SFML::Network
    SFML::System
    SFML::Main
    ImGui-SFML::ImGui-SFML
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})