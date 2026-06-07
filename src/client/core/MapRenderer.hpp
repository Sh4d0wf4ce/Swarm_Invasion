#pragma once

#include "MapGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

class MapRenderer{
public:
    MapRenderer(std::shared_ptr<MapGenerator> mapGen, float tileSize);
    void render(sf::RenderTarget& target);
    void rebuild();

private:
    // Map Data
    std::shared_ptr<MapGenerator> m_mapGen;
    float m_tileSize;

    // GPU Cache
    sf::VertexArray m_vertices;
};
