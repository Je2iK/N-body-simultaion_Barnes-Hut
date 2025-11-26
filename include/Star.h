#ifndef STAR_H
#define STAR_H

#include <SFML/Graphics.hpp>
#include <cmath>

struct Star {
    double x, y;
    double vx, vy;
    double mass;
    sf::Color color;
    float radius;
    
    Star(double x = 0, double y = 0, double vx = 0, double vy = 0, 
         double mass = 1, sf::Color color = sf::Color::White)
        : x(x), y(y), vx(vx), vy(vy), mass(mass), color(color)
    {
        radius = std::max(1.0f, static_cast<float>(0.5 + std::log(mass + 1) * 0.3));
    }
};

#endif // STAR_H
