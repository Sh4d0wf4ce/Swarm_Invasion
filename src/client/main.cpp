#include "ClientEngine.hpp"
#include <iostream>

int main(){
    try{
        std::cout<<"Starting Swarm Invasion Client...\n";

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