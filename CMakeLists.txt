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
    src/shared/NetworkProtocol.cpp
)
target_include_directories(SharedStatic PUBLIC src/shared)
target_link_libraries(SharedStatic PUBLIC nlohmann_json::nlohmann_json)

add_executable(SwarmServer
    src/server/main.cpp
    src/server/ServerEngine.cpp
)
target_link_libraries(SwarmServer PRIVATE SharedStatic SFML::Network SFML::System SFML::Main)

add_executable(SwarmClient
    src/client/main.cpp
    src/client/ClientEngine.cpp
    src/client/Entity.cpp
    src/client/Player.cpp
    src/client/Enemy.cpp
    src/Client/MapRenderer.cpp
    src/Client/ProjectileManager.cpp
    src/Client/Projectile.cpp
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