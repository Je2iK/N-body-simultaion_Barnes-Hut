#ifndef CELL_H
#define CELL_H

#include "Star.h"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

// Класс для представления клетки в дереве (Quadtree)
class Cell {
public:
    double x, y;
    double width, height;
    double mass;
    double com_x, com_y;  // Center of mass
    
    std::vector<Star*> stars;
    std::vector<std::unique_ptr<Cell>> children;
    bool is_divided;

    Cell(double x, double y, double width, double height);
    
    void draw(sf::RenderWindow& window, float scale) const;
};

#endif // CELL_H
