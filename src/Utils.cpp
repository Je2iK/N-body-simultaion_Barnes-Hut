#include "Utils.h"

std::vector<Star> createGalaxy(double galaxy_radius, double center_x, double center_y, 
                               double acceleration_factor,
                               double galaxy_vx, double galaxy_vy) {
    using namespace Physics;
    using namespace Visual;
    using namespace Simulation;
    
    std::vector<Star> stars;
    stars.reserve(NUM_STARS);
    
    // Добавление черной дыры (stars[0]) с начальной скоростью галактики
    stars.emplace_back(center_x, center_y, galaxy_vx, galaxy_vy, BLACK_HOLE_MASS, sf::Color::White);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(0.0, 1.0);
    std::uniform_real_distribution<> mass_dist(0.5, 1.5);
    std::uniform_int_distribution<> color_dist(0, STAR_COLORS.size() - 1);
    
    for (int i = 0; i < NUM_STARS_EXCEPT_BH; ++i) {
        const double r = galaxy_radius * std::sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        
        const double x = center_x + r * std::cos(angle);
        const double y = center_y + r * std::sin(angle);
        
        const double mass = mass_dist(gen);
        
        // Расчет начальной скорости
        const double r_dist_smooth = std::sqrt(r * r + EPSILON_SQ);
        const double orbital_speed_ideal = std::sqrt(G * BLACK_HOLE_MASS / r_dist_smooth);
        
        const double orbital_speed = orbital_speed_ideal * acceleration_factor;
        
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx = -orbital_speed * std::sin(angle) * noise + galaxy_vx; // Добавляем скорость галактики
        const double vy = orbital_speed * std::cos(angle) * noise + galaxy_vy;  // Добавляем скорость галактики
        
        stars.emplace_back(x, y, vx, vy, mass, STAR_COLORS[color_dist(gen)]);
    }
    
    return stars;
}

std::vector<Star> createGalaxyCollision(double galaxy_radius, double area_width, double area_height) {
    using namespace Physics;
    using namespace Visual;
    using namespace Simulation;
    
    std::vector<Star> stars;
    
    // Параметры для двух галактик
    const int stars_per_galaxy = NUM_STARS_EXCEPT_BH / 2;
    const double separation = area_width * 0.4;  // Уменьшено расстояние (было 0.6)
    
    // Центры галактик
    const double galaxy1_x = area_width / 2 - separation / 2;
    const double galaxy1_y = area_height / 2;
    const double galaxy2_x = area_width / 2 + separation / 2;
    const double galaxy2_y = area_height / 2;
    
    // Скорости сближения (направлены друг к другу)
    const double collision_speed = 1.5;  // Увеличена скорость (было 0.5)
    const double galaxy1_vx = collision_speed;
    const double galaxy1_vy = 0.0;
    const double galaxy2_vx = -collision_speed;
    const double galaxy2_vy = 0.0;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(0.0, 1.0);
    std::uniform_real_distribution<> mass_dist(0.5, 1.5);
    
    // === ГАЛАКТИКА 1 (синяя) ===
    // Черная дыра галактики 1
    stars.emplace_back(galaxy1_x, galaxy1_y, galaxy1_vx, galaxy1_vy, 
                      BLACK_HOLE_MASS, sf::Color::Cyan);
    
    for (int i = 0; i < stars_per_galaxy; ++i) {
        const double r = galaxy_radius * std::sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        
        const double x = galaxy1_x + r * std::cos(angle);
        const double y = galaxy1_y + r * std::sin(angle);
        
        const double mass = mass_dist(gen);
        
        // Орбитальная скорость вокруг черной дыры + скорость галактики
        const double r_dist_smooth = std::sqrt(r * r + EPSILON_SQ);
        const double orbital_speed = std::sqrt(G * BLACK_HOLE_MASS / r_dist_smooth) * 0.99;
        
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx_orbital = -orbital_speed * std::sin(angle) * noise;
        const double vy_orbital = orbital_speed * std::cos(angle) * noise;
        
        // Добавляем скорость галактики
        const double vx = vx_orbital + galaxy1_vx;
        const double vy = vy_orbital + galaxy1_vy;
        
        stars.emplace_back(x, y, vx, vy, mass, sf::Color(100, 150, 255)); // Голубые звезды
    }
    
    // === ГАЛАКТИКА 2 (красная) ===
    // Черная дыра галактики 2
    stars.emplace_back(galaxy2_x, galaxy2_y, galaxy2_vx, galaxy2_vy, 
                      BLACK_HOLE_MASS, sf::Color::Red);
    
    for (int i = 0; i < stars_per_galaxy; ++i) {
        const double r = galaxy_radius * std::sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        
        const double x = galaxy2_x + r * std::cos(angle);
        const double y = galaxy2_y + r * std::sin(angle);
        
        const double mass = mass_dist(gen);
        
        // Орбитальная скорость вокруг черной дыры + скорость галактики
        const double r_dist_smooth = std::sqrt(r * r + EPSILON_SQ);
        const double orbital_speed = std::sqrt(G * BLACK_HOLE_MASS / r_dist_smooth) * 0.99;
        
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx_orbital = -orbital_speed * std::sin(angle) * noise;
        const double vy_orbital = orbital_speed * std::cos(angle) * noise;
        
        // Добавляем скорость галактики
        const double vx = vx_orbital + galaxy2_vx;
        const double vy = vy_orbital + galaxy2_vy;
        
        stars.emplace_back(x, y, vx, vy, mass, sf::Color(255, 100, 100)); // Красные звезды
    }
    
    return stars;
}

void drawStars(sf::RenderWindow& window, const std::vector<Star>& stars, float scale) {
    sf::VertexArray vertices(sf::Points, stars.size());
    
    for (size_t i = 0; i < stars.size(); ++i) {
        const auto& star = stars[i];
        vertices[i].position = sf::Vector2f(static_cast<float>(star.x * scale),
                                           static_cast<float>(star.y * scale));
        
        // Проверяем, является ли это черной дырой (по массе)
        if (star.mass >= Physics::BLACK_HOLE_MASS * 0.9) {
            // Отрисовка черной дыры
            sf::CircleShape shape(3.0f);
            shape.setFillColor(star.color);  // Используем цвет звезды
            shape.setOutlineColor(sf::Color::White);
            shape.setOutlineThickness(2.0f);
            shape.setOrigin(3.0f, 3.0f);
            shape.setPosition(vertices[i].position);
            window.draw(shape);
            
            vertices[i].color = sf::Color::Transparent;
        } else {
            vertices[i].color = star.color;
        }
    }
    
    window.draw(vertices);
}

