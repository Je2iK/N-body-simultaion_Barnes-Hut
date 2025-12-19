#ifndef STAR_H
#define STAR_H
#include <SFML/Graphics.hpp>
#include <cmath>
#include <deque>
using namespace std;
using namespace sf;
struct Star {
    double x, y;
    double vx, vy;
    double mass;
    Color color;
    float radius;
    deque<Vector2f> trail;
    Star(double x = 0, double y = 0, double vx = 0, double vy = 0, 
         double mass = 1, Color color = Color::White)
        : x(x), y(y), vx(vx), vy(vy), mass(mass), color(color)
    {
        radius = max(1.0f, static_cast<float>(0.5 + log(mass + 1) * 0.3));
    }
};
#endif 
