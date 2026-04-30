#include "ServerEngine.hpp"
#include <iostream>

int main(){
    try{
        std::cout<<"---SWARM INVASION - SERVER ---\n";

        ServerEngine engine;
        engine.run();
    }
    catch(const std::exception& e){
        std::cerr<<"[SERVER] Error: "<<e.what()<<"\n";
        return 1;
    }

    return 0;
}