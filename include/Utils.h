#ifndef UTILS_H
#define UTILS_H

#include "Star.h"
#include "Constants.h"
#include <vector>
#include <random>
#include <SFML/Graphics.hpp>

// Быстрая обратная величина квадратного корня
inline double fast_inv_sqrt(double x) {
    double y = x;
    double x2 = y * 0.5;
    long long i = *(long long*)&y;
    i = 0x5FE6EB50C7B537A9 - (i >> 1);
    y = *(double*)&i;
    return y * (1.5 - (x2 * y * y));
}

// Создание одной галактики
std::vector<Star> createGalaxy(double galaxy_radius, double center_x, double center_y, 
                               double acceleration_factor = 0.99,
                               double galaxy_vx = 0.0, double galaxy_vy = 0.0);

// Создание столкновения двух галактик
std::vector<Star> createGalaxyCollision(double galaxy_radius, double area_width, double area_height);

// Отрисовка звезд
void drawStars(sf::RenderWindow& window, const std::vector<Star>& stars, float scale);

#endif // UTILS_H
