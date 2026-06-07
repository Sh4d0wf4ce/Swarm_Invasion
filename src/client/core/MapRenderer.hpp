#pragma once

#include "MapGenerator.hpp"

#include <SFML/Graphics.hpp>
#include <memory>

/**
 * @brief GPU-cached tile map renderer.
 *
 * Builds a vertex array from a MapGenerator and draws the visible map
 * tiles as colored triangles. Call rebuild() when the underlying map data
 * changes.
 */
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
