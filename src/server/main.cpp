#include "ServerEngine.hpp"
#include "WeaponRegistry.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include <iostream>

int main(){
    try{
        std::cout<<"---SWARM INVASION - SERVER ---\n";
        WeaponRegistry::loadConfig("assets/weapons.json");
        HeroRegistry::loadConfig("assets/heroes.json");
        EnemyRegistry::loadConfig("assets/enemies.json");
        ServerEngine engine;
        engine.run();
    }
    catch(const std::exception& e){
        std::cerr<<"[SERVER] Error: "<<e.what()<<"\n";
        return 1;
    }

    return 0;
}