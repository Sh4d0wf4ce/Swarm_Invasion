#pragma once

#include "MapGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

class MapRenderer{
public:
    MapRenderer(std::shared_ptr<MapGenerator> mapGen, float tileSize);
    void render(sf::RenderTarget& target);

private:
    std::shared_ptr<MapGenerator> m_mapGen;
    float m_tileSize;

    sf::RectangleShape m_wallShape;
    sf::RectangleShape m_floorShape;
};