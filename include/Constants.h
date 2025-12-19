#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <thread>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Физика
namespace Physics {
    const double G = 0.001;
    const double EPSILON_SQ = 25.0;
    const double BLACK_HOLE_MASS = 1000000.0;
    const double BLACK_HOLE_X = 400.0;
    const double BLACK_HOLE_Y = 400.0;
}

// Barnes-Hut
namespace BarnesHut {
    const double DT = 0.5; 
    const double THETA = 0.5;
    const double MIN_CELL_SIZE = 10.0;
}

// Brute Force
namespace BruteForce {
    const double DT = 0.5; 
}

// Симуляция
namespace Simulation {
    const int NUM_STARS_EXCEPT_BH = 500000;
    const int NUM_STARS = NUM_STARS_EXCEPT_BH + 1;
    const int NUM_THREADS = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    const int NUM_STEPS_BENCHMARK = 100;  }

// Визуализация
namespace Visual {
    const sf::Color BACKGROUND_COLOR(18, 18, 18);
    const sf::Color TREE_COLOR(30, 215, 96, 180);
    const std::vector<sf::Color> STAR_COLORS = {
        sf::Color::White,
        sf::Color(255, 100, 100),
        sf::Color(255, 230, 160),
        sf::Color(150, 200, 255)
    };
}

#endif // CONSTANTS_H
