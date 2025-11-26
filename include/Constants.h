#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <thread>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- ФИЗИЧЕСКИЕ КОНСТАНТЫ ---
namespace Physics {
    const double G = 0.001;              // Гравитационная постоянная
    const double EPSILON_SQ = 25.0;      // Сглаживание (уменьшено для более точных сил)
    const double BLACK_HOLE_MASS = 1000000.0;
    const double BLACK_HOLE_X = 400.0;
    const double BLACK_HOLE_Y = 400.0;
}

// --- КОНСТАНТЫ BARNES-HUT ---
namespace BarnesHut {
    const double DT = 0.1;               // Уменьшенный шаг времени для стабильности
    const double THETA = 0.5;            // Более точный критерий Barnes-Hut
    const double MIN_CELL_SIZE = 10.0;
}

// --- КОНСТАНТЫ BRUTE FORCE ---
namespace BruteForce {
    const double DT = 2.0;
}

// --- КОНСТАНТЫ СИМУЛЯЦИИ ---
namespace Simulation {
    const int NUM_STARS_EXCEPT_BH = 5000;
    const int NUM_STARS = NUM_STARS_EXCEPT_BH + 1;
    const int NUM_THREADS = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    const int NUM_STEPS_BENCHMARK = 1000;
}

// --- КОНСТАНТЫ ВИЗУАЛИЗАЦИИ ---
namespace Visual {
    const sf::Color BACKGROUND_COLOR(5, 5, 15);
    const sf::Color TREE_COLOR(1, 1, 1, 1);
    const std::vector<sf::Color> STAR_COLORS = {
        sf::Color::White,
        sf::Color::Red,
        sf::Color(255, 230, 160),
        sf::Color(200, 200, 255)
    };
}

#endif // CONSTANTS_H
