#ifndef UTILS_H
#define UTILS_H

#include "Star.h"
#include "Constants.h"
#include <vector>
#include <random>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

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
vector<Star> createGalaxy(int num_stars, double galaxy_radius, double center_x, double center_y, 
                               double acceleration_factor = 0.99,
                               double galaxy_vx = 0.0, double galaxy_vy = 0.0);

// Создание столкновения двух галактик
vector<Star> createGalaxyCollision(int num_stars, double galaxy_radius, double area_width, double area_height);

// Отрисовка звезд
void drawStars(RenderWindow& window, const vector<Star>& stars, float scale, bool showTrails = false);

// Загрузка шрифта (кроссплатформенная)
bool loadFont(Font& font);

#endif // UTILS_H
