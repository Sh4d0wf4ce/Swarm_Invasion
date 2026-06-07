#include "ClientEngine.hpp"
#include "WeaponRegistry.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include "UpgradeRegistry.hpp"
#include "AbilityRegistry.hpp"
#include <iostream>

/**
 * @brief Application entry point for the Swarm Invasion client.
 * @return 0 on normal exit, 1 if an unhandled exception occurs.
 */
int main(){
    try{
        // --- Load game data registries ---
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        std::cout<<"Starting Swarm Invasion Client...\n";
        WeaponRegistry::loadConfig("assets/weapons.json");
        HeroRegistry::loadConfig("assets/heroes.json");
        EnemyRegistry::loadConfig("assets/enemies.json");
        UpgradeRegistry::loadConfig("assets/upgrades.json");
        AbilityRegistry::loadConfig("assets/abilities.json");

        // --- Run client engine ---
        ClientEngine engine;
        engine.run();
        std::cout<<"Client has been closed";
    }
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
