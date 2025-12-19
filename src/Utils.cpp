#include "Utils.h"
using namespace std;
using namespace sf;
vector<Star> createGalaxy(int num_stars, double galaxy_radius, double center_x, double center_y, 
                               double acceleration_factor,
                               double galaxy_vx, double galaxy_vy) {
    using namespace Physics;
    using namespace Visual;
    using namespace Simulation;
    vector<Star> stars;
    stars.reserve(num_stars);
    stars.emplace_back(center_x, center_y, galaxy_vx, galaxy_vy, BLACK_HOLE_MASS, Color::White);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> pos_dist(0.0, 1.0);
    uniform_real_distribution<> mass_dist(0.5, 1.5);
    uniform_int_distribution<> color_dist(0, STAR_COLORS.size() - 1);
    for (int i = 0; i < num_stars - 1; ++i) {
        const double r = galaxy_radius * sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        const double x = center_x + r * cos(angle);
        const double y = center_y + r * sin(angle);
        const double mass = mass_dist(gen);
        const double r_dist_smooth = sqrt(r * r + EPSILON_SQ);
        const double orbital_speed_ideal = sqrt(G * BLACK_HOLE_MASS / r_dist_smooth);
        const double orbital_speed = orbital_speed_ideal * acceleration_factor;
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx = -orbital_speed * sin(angle) * noise + galaxy_vx;
        const double vy = orbital_speed * cos(angle) * noise + galaxy_vy;
        stars.emplace_back(x, y, vx, vy, mass, STAR_COLORS[color_dist(gen)]);
    }
    return stars;
}
vector<Star> createGalaxyCollision(int num_stars, double galaxy_radius, double area_width, double area_height) {
    using namespace Physics;
    using namespace Visual;
    using namespace Simulation;
    vector<Star> stars;
    const int stars_per_galaxy = (num_stars - 2) / 2;
    const double separation = area_width * 0.4;
    const double galaxy1_x = area_width / 2 - separation / 2;
    const double galaxy1_y = area_height / 2;
    const double galaxy2_x = area_width / 2 + separation / 2;
    const double galaxy2_y = area_height / 2;
    const double collision_speed = 1.5;  
    const double galaxy1_vx = collision_speed;
    const double galaxy1_vy = 0.0;
    const double galaxy2_vx = -collision_speed;
    const double galaxy2_vy = 0.0;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> pos_dist(0.0, 1.0);
    uniform_real_distribution<> mass_dist(0.5, 1.5);
    stars.emplace_back(galaxy1_x, galaxy1_y, galaxy1_vx, galaxy1_vy, 
                      BLACK_HOLE_MASS, Color::Cyan);
    for (int i = 0; i < stars_per_galaxy; ++i) {
        const double r = galaxy_radius * sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        const double x = galaxy1_x + r * cos(angle);
        const double y = galaxy1_y + r * sin(angle);
        const double mass = mass_dist(gen);
        const double r_dist_smooth = sqrt(r * r + EPSILON_SQ);
        const double orbital_speed = sqrt(G * BLACK_HOLE_MASS / r_dist_smooth) * 0.99;
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx_orbital = -orbital_speed * sin(angle) * noise;
        const double vy_orbital = orbital_speed * cos(angle) * noise;
        const double vx = vx_orbital + galaxy1_vx;
        const double vy = vy_orbital + galaxy1_vy;
        stars.emplace_back(x, y, vx, vy, mass, Color(100, 150, 255));
    }
    stars.emplace_back(galaxy2_x, galaxy2_y, galaxy2_vx, galaxy2_vy, 
                      BLACK_HOLE_MASS, Color::Red);
    for (int i = 0; i < stars_per_galaxy; ++i) {
        const double r = galaxy_radius * sqrt(pos_dist(gen));
        const double angle = 2.0 * M_PI * pos_dist(gen);
        const double x = galaxy2_x + r * cos(angle);
        const double y = galaxy2_y + r * sin(angle);
        const double mass = mass_dist(gen);
        const double r_dist_smooth = sqrt(r * r + EPSILON_SQ);
        const double orbital_speed = sqrt(G * BLACK_HOLE_MASS / r_dist_smooth) * 0.99;
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2;
        const double vx_orbital = -orbital_speed * sin(angle) * noise;
        const double vy_orbital = orbital_speed * cos(angle) * noise;
        const double vx = vx_orbital + galaxy2_vx;
        const double vy = vy_orbital + galaxy2_vy;
        stars.emplace_back(x, y, vx, vy, mass, Color(255, 100, 100));
    }
    return stars;
}
void drawStars(RenderWindow& window, const vector<Star>& stars, float scale, bool showTrails) {
    if (showTrails) {
        VertexArray trails(PrimitiveType::Lines);
        for (const auto& star : stars) {
            if (star.trail.size() < 2) continue;
            for (size_t i = 0; i < star.trail.size() - 1; ++i) {
                Vector2f p1 = star.trail[i] * scale;
                Vector2f p2 = star.trail[i+1] * scale;
                Color trailColor = star.color;
                float alpha = 255.0f * (static_cast<float>(i) / star.trail.size());
                trailColor.a = static_cast<uint8_t>(alpha * 0.5f);
                Vertex v1, v2;
                v1.position = p1;
                v1.color = trailColor;
                v2.position = p2;
                v2.color = trailColor;
                trails.append(v1);
                trails.append(v2);
            }
        }
        window.draw(trails);
    }
    VertexArray vertices(PrimitiveType::Points, stars.size());
    for (size_t i = 0; i < stars.size(); ++i) {
        const auto& star = stars[i];
        vertices[i].position = Vector2f(static_cast<float>(star.x * scale),
                                           static_cast<float>(star.y * scale));
        if (star.mass >= Physics::BLACK_HOLE_MASS * 0.9) {
            CircleShape shape(3.0f);
            shape.setFillColor(star.color);
            shape.setOutlineColor(Color::White);
            shape.setOutlineThickness(2.0f);
            shape.setOrigin({3.0f, 3.0f});
            shape.setPosition(vertices[i].position);
            window.draw(shape);
            vertices[i].color = Color::Transparent;
        } else {
            vertices[i].color = star.color;
        }
    }
    window.draw(vertices);
}
bool loadFont(Font& font) {
    vector<string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "arial.ttf"
    };
    for (const auto& path : fontPaths) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}
