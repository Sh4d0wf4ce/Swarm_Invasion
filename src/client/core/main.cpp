#include "ClientEngine.hpp"
#include "WeaponRegistry.hpp"
#include "HeroRegistry.hpp"
#include "EnemyRegistry.hpp"
#include <iostream>

int main(){
    try{
        std::cout<<"Starting Swarm Invasion Client...\n";

        WeaponRegistry::loadConfig("assets/weapons.json");
        HeroRegistry::loadConfig("assets/heroes.json");
        EnemyRegistry::loadConfig("assets/enemies.json");
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