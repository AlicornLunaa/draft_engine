#include "world.hpp"
#include <cstddef>

using namespace Krypton::Simulation;

World::World(unsigned int width, unsigned int height) : width(width), height(height) {
    cellArray = new Cell*[width * height];

    for(size_t i = 0; i < width * height; i++){
        unsigned int x = i % width;
        unsigned int y = i / width;
        cellArray[i] = new Cell(CellType::EMPTY, x, y);
    }

    shape = sf::RectangleShape({ cellSize, cellSize });
}

World::~World(){
    // Delete every cell
    for(size_t i = 0; i < width * height; i++)
        delete cellArray[i];

    delete[] cellArray;
}

void World::draw(sf::RenderWindow& window){
    for(size_t i = 0; i < width * height; i++){
        unsigned int x = i % width;
        unsigned int y = i / width;

        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x * (cellSize * 2 + 0.01f), y * (cellSize * 2 + 0.01f));
        window.draw(shape);
    }
}