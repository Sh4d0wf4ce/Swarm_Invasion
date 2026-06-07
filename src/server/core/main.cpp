#include "ServerEngine.hpp"
#include "WeaponRegistry.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include "UpgradeRegistry.hpp"
#include "AbilityRegistry.hpp"
#include <iostream>

/**
 * @brief Server entry point: loads game data registries and runs the UDP game loop.
 * @return 0 on success, 1 if an unhandled exception occurs during startup or execution.
 * @throws None Exceptions are caught internally and reported to stderr.
 */
int main(){
    try{
        // --- Load game data registries ---
        std::cout<<"---SWARM INVASION - SERVER ---\n";
        WeaponRegistry::loadConfig("assets/weapons.json");
        HeroRegistry::loadConfig("assets/heroes.json");
        EnemyRegistry::loadConfig("assets/enemies.json");
        UpgradeRegistry::loadConfig("assets/upgrades.json");
        AbilityRegistry::loadConfig("assets/abilities.json");

        // --- Run server loop ---
        ServerEngine engine;
        engine.run();
    }
    catch(const std::exception& e){
        std::cerr<<"[SERVER] Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
