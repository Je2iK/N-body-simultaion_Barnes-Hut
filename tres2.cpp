#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <SFML/Graphics.hpp>

// Математические константы
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- КОНСТАНТЫ ФИЗИКИ ---
const double G = 0.001;      
const double DT = 4.3;      // СНИЖЕННЫЙ ШАГ ВРЕМЕНИ (для точности БХ)
const double THETA = 1.0;    // ОЧЕНЬ МАЛЫЙ КРИТЕРИЙ ТЕТА (Высокая точность аппроксимации)
const double EPSILON_SQ = 100.0; // ЭКСТРЕМАЛЬНОЕ СГЛАЖИВАНИЕ
const double BLACK_HOLE_MASS = 1000000.0; 
const double BLACK_HOLE_X = 400.0;
const double BLACK_HOLE_Y = 400.0;
const double MIN_CELL_SIZE = 400.0;

// --- КОНСТАНТЫ СИМУЛЯЦИИ ---
const int NUM_STARS_EXCEPT_BH = 100000;
const int NUM_STARS = NUM_STARS_EXCEPT_BH + 1; 
const int NUM_THREADS = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
const int NUM_STEPS_BENCHMARK = 10000; // Количество шагов для бенчмарка
const std::string BENCHMARK_FILE = "benchmark_barnes_hut.txt"; // Файл для сохранения результатов

// --- КОНСТАНТЫ ВИЗУАЛИЗАЦИИ ---
const sf::Color BACKGROUND_COLOR(5, 5, 15);
const sf::Color TREE_COLOR(1, 1, 1, 1);
const std::vector<sf::Color> STAR_COLORS = {
    sf::Color::White
};

// Быстрая обратная величина квадратного корня
inline double fast_inv_sqrt(double x) {
    double y = x;
    double x2 = y * 0.5;
    long long i = *(long long*)&y;
    i = 0x5FE6EB50C7B537A9 - (i >> 1);
    y = *(double*)&i;
    return y * (1.5 - (x2 * y * y));
}

// Класс для представления звезды/объекта
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

// Класс для представления клетки в дереве (Quadtree)
class Cell {
public:
    double x, y;          
    double width, height; 
    double mass;          
    double com_x, com_y;  
    
    std::vector<Star*> stars;
    std::vector<std::unique_ptr<Cell>> children;
    bool is_divided;

    Cell(double x, double y, double width, double height)
        : x(x), y(y), width(width), height(height), 
          mass(0), com_x(0), com_y(0), is_divided(false) 
    {
        stars.reserve(1);
    }
    
    void draw(sf::RenderWindow& window, float scale) const {
        if (stars.empty() && children.empty()) return;
        
        if (width * scale < 5.0f && !is_divided) return;
        
        sf::RectangleShape rect(sf::Vector2f(static_cast<float>(width * scale), 
                                            static_cast<float>(height * scale)));
        rect.setPosition(static_cast<float>((x - width/2) * scale), 
                        static_cast<float>((y - height/2) * scale));
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(TREE_COLOR);
        rect.setOutlineThickness(0.3f);
        window.draw(rect);
        
        for (const auto& child : children) {
            child->draw(window, scale);
        }
    }
};

// Основной класс симулятора
class GalaxySimulator {
private:
    std::unique_ptr<Cell> root;
    double area_width, area_height;
    bool show_tree = false; 
    std::atomic<bool> tree_built{false};
    
public:
    GalaxySimulator(double width, double height) 
        : area_width(width), area_height(height)
    {
        resetTree();
    }
    
    void resetTree() {
        root = std::make_unique<Cell>(area_width/2, area_height/2, area_width, area_height);
        tree_built = false;
    }
    
    void buildTree(const std::vector<Star>& stars) {
        resetTree();
        for (const auto& star : stars) { 
            insert(&star);
        }
        tree_built = true;
    }
    
    void recalculateCellMassAndCOM(Cell* cell) {
        cell->mass = 0.0;
        cell->com_x = 0.0;
        cell->com_y = 0.0;
        
        if (cell->stars.empty()) return;
        
        double total_mass = 0.0;
        double sum_x = 0.0;
        double sum_y = 0.0;
        
        for (const auto* star : cell->stars) {
            sum_x += star->x * star->mass;
            sum_y += star->y * star->mass;
            total_mass += star->mass;
        }
        
        cell->mass = total_mass;
        if (total_mass > 0) {
            cell->com_x = sum_x / total_mass;
            cell->com_y = sum_y / total_mass;
        } else if (!cell->stars.empty()) {
            cell->com_x = cell->stars[0]->x;
            cell->com_y = cell->stars[0]->y;
        }
    }
    
    void updateCellMassAndCOM(Cell* cell, const Star* star) {
        const double old_mass = cell->mass;
        const double new_mass = cell->mass + star->mass;
        
        if (new_mass > 0) {
            cell->com_x = (cell->com_x * old_mass + star->x * star->mass) / new_mass;
            cell->com_y = (cell->com_y * old_mass + star->y * star->mass) / new_mass;
        } else {
             cell->com_x = star->x;
             cell->com_y = star->y;
        }
        cell->mass = new_mass;
    }
    
    int getQuadrant(const Star* star, const Cell* cell) const {
        if (star->x < cell->x) {
            return (star->y < cell->y) ? 0 : 2;
        } else {
            return (star->y < cell->y) ? 1 : 3;
        }
    }

    void subdivide(Cell* cell) {
        if (cell->width < MIN_CELL_SIZE || cell->height < MIN_CELL_SIZE) {
            return;
        }
        
        const double half_width = cell->width / 2;
        const double half_height = cell->height / 2;
        const double quarter_width = half_width / 2;
        const double quarter_height = half_height / 2;
        
        cell->children.push_back(std::make_unique<Cell>(
            cell->x - quarter_width, cell->y - quarter_height, half_width, half_height));
        cell->children.push_back(std::make_unique<Cell>(
            cell->x + quarter_width, cell->y - quarter_height, half_width, half_height));
        cell->children.push_back(std::make_unique<Cell>(
            cell->x - quarter_width, cell->y + quarter_height, half_width, half_height));
        cell->children.push_back(std::make_unique<Cell>(
            cell->x + quarter_width, cell->y + quarter_height, half_width, half_height));
        
        cell->is_divided = true;
        
        auto old_stars = std::move(cell->stars);
        cell->stars.clear(); 
        
        for (auto* star : old_stars) {
            const int quadrant = getQuadrant(star, cell);
            if (quadrant >= 0 && quadrant < 4) {
                cell->children[quadrant]->stars.push_back(star);
            }
        }
        
        for (const auto& child : cell->children) {
             recalculateCellMassAndCOM(child.get());
        }
    }

    void insert(const Star* star) {
        Cell* current = root.get();
        updateCellMassAndCOM(current, star); 

        while (true) {
            if (!current->is_divided) {
                
                if (current->stars.empty()) {
                    current->stars.push_back(const_cast<Star*>(star));
                    return;
                    
                } else if (current->stars.size() == 1) { 
                    subdivide(current);
                    
                    if (!current->is_divided) {
                        current->stars.push_back(const_cast<Star*>(star));
                        recalculateCellMassAndCOM(current);
                        return;
                    }
                }
            }
            
            if (current->is_divided) {
                const int quadrant = getQuadrant(star, current);
                current = current->children[quadrant].get();
                updateCellMassAndCOM(current, star); 
            } else {
                 return;
            }
        }
    }
    
    // Единый расчет ускорения (Barnes-Hut)
    std::pair<double, double> calculateTreeAccelerationRecursive(const Star* star, const Cell* cell) const {
        if (cell == nullptr || cell->mass == 0.0) return {0, 0};
        
        const double dx_com = cell->com_x - star->x;
        const double dy_com = cell->com_y - star->y;
        const double distance_sq_com = dx_com*dx_com + dy_com*dy_com + EPSILON_SQ;
        const double distance_com = std::sqrt(distance_sq_com);

        // Критерий Барнса-Хата (Уменьшен для точности)
        if (!cell->is_divided || (cell->width / distance_com) < THETA) 
        {
            
            // Проверка на расчет силы на саму себя
            if (cell->stars.size() == 1 && cell->stars[0] == star) {
                 return {0, 0};
            }

            // Аппроксимация через Центр Масс
            const double inv_distance_cubed = fast_inv_sqrt(distance_sq_com) * fast_inv_sqrt(distance_sq_com) * fast_inv_sqrt(distance_sq_com);
            const double acceleration_magnitude = G * cell->mass * inv_distance_cubed;
            
            return {acceleration_magnitude * dx_com, acceleration_magnitude * dy_com};
            
        } else {
            // Рекурсия 
            double acc_x = 0.0, acc_y = 0.0;
            for (const auto& child : cell->children) {
                auto [fx, fy] = calculateTreeAccelerationRecursive(star, child.get());
                acc_x += fx;
                acc_y += fy;
            }
            return {acc_x, acc_y};
        }
    }
    
    // Многопоточный расчет ускорений
    void calculateAccelerationsParallel(const std::vector<Star>& stars, 
                               std::vector<double>& acc_x, 
                               std::vector<double>& acc_y) const {
        if (!tree_built) return;
        
        const size_t start_index = 1; 
        const size_t num_particles = stars.size() - start_index;

        if (num_particles == 0) return;
        
        const size_t chunk_size = (num_particles + NUM_THREADS - 1) / NUM_THREADS;
        std::vector<std::thread> threads;
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            const size_t start = start_index + i * chunk_size;
            const size_t end = std::min(start + chunk_size, stars.size());
            
            if (start < end) {
                threads.emplace_back([&, start, end]() {
                    for (size_t j = start; j < end; ++j) {
                        auto [tree_ax, tree_ay] = calculateTreeAccelerationRecursive(&stars[j], root.get());
                        
                        acc_x[j] = tree_ax;
                        acc_y[j] = tree_ay;
                    }
                });
            }
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
    
    // ИНТЕГРИРОВАНИЕ LEAPFROG
    void timeStep(std::vector<Star>& stars) {
        
        std::vector<double> acc_x(stars.size(), 0.0);
        std::vector<double> acc_y(stars.size(), 0.0);
        
        // Шаг 1: Расчет ускорений a(t)
        buildTree(stars);
        calculateAccelerationsParallel(stars, acc_x, acc_y); 

        std::vector<double> vx_half(stars.size());
        std::vector<double> vy_half(stars.size());
        
        // Шаг 2: V(t + 1/2) и X(t + 1)
        
        const size_t start_index = 1; 
        const size_t num_particles = stars.size() - start_index;

        if (num_particles == 0) return;
        
        const size_t chunk_size = (num_particles + NUM_THREADS - 1) / NUM_THREADS;
        std::vector<std::thread> update_threads;
        
        for (int i = 0; i < NUM_THREADS; ++i) {
            const size_t start = start_index + i * chunk_size;
            const size_t end = std::min(start + chunk_size, stars.size());
            
            if (start < end) {
                update_threads.emplace_back([&, start, end]() {
                    for (size_t j = start; j < end; ++j) {
                        const double ax = acc_x[j];
                        const double ay = acc_y[j];
                        
                        vx_half[j] = stars[j].vx + ax * DT * 0.5;
                        vy_half[j] = stars[j].vy + ay * DT * 0.5;
                        
                        stars[j].x += vx_half[j] * DT;
                        stars[j].y += vy_half[j] * DT;
                        
                        // Обработка границ
                        if (stars[j].x < 0) stars[j].x += area_width;
                        if (stars[j].x >= area_width) stars[j].x -= area_width;
                        if (stars[j].y < 0) stars[j].y += area_height;
                        if (stars[j].y >= area_height) stars[j].y -= area_height;
                    }
                });
            }
        }
        
        for (auto& thread : update_threads) {
            thread.join();
        }
        update_threads.clear();


        // Шаг 3: Пересчет ускорений a(t + 1) по новым позициям
        buildTree(stars); 
        calculateAccelerationsParallel(stars, acc_x, acc_y); 
        

        // Шаг 4: V(t + 1) - Обновление скорости на оставшийся полшага
        for (int i = 0; i < NUM_THREADS; ++i) {
            const size_t start = start_index + i * chunk_size;
            const size_t end = std::min(start + chunk_size, stars.size());
            
            if (start < end) {
                update_threads.emplace_back([&, start, end]() {
                    for (size_t j = start; j < end; ++j) {
                        const double ax_new = acc_x[j];
                        const double ay_new = acc_y[j];
                        
                        stars[j].vx = (vx_half[j] + ax_new * DT * 0.5);
                        stars[j].vy = (vy_half[j] + ay_new * DT * 0.5);
                    }
                });
            }
        }
        
        for (auto& thread : update_threads) {
            thread.join();
        }
    }
    
    void drawTree(sf::RenderWindow& window, float scale) const {
        if (show_tree && tree_built) {
            root->draw(window, scale);
        }
    }
    
    void toggleTree() { show_tree = !show_tree; }
};

// Быстрое создание галактики С УСИЛЕННЫМИ НАЧАЛЬНЫМИ СКОРОСТЯМИ
std::vector<Star> createGalaxy(double galaxy_radius, double center_x, double center_y) {
    std::vector<Star> stars;
    stars.reserve(NUM_STARS);
    
    // 1. ДОБАВЛЕНИЕ ЧЕРНОЙ ДЫРЫ (stars[0])
    stars.emplace_back(center_x, center_y, 0.0, 0.0, BLACK_HOLE_MASS, sf::Color::White);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(0.0, 1.0);
    std::uniform_real_distribution<> mass_dist(0.5, 1.5);
    std::uniform_int_distribution<> color_dist(0, STAR_COLORS.size() - 1);
    
    const double acceleration_factor = 0.99; // Коэффициент начальной скорости
    
    for (int i = 0; i < NUM_STARS_EXCEPT_BH; ++i) {
        // Распределение по радиусу (более плотное в центре)
        const double r = galaxy_radius * std::sqrt(pos_dist(gen)); 
        const double angle = 2.0 * M_PI * pos_dist(gen);
        
        const double x = center_x + r * std::cos(angle);
        const double y = center_y + r * std::sin(angle);
        
        const double mass = mass_dist(gen);
        
        // Расчет начальной скорости
        const double r_dist_smooth = std::sqrt(r * r + EPSILON_SQ); 
        const double orbital_speed_ideal = std::sqrt(G * BLACK_HOLE_MASS / r_dist_smooth); 
        
        const double orbital_speed = orbital_speed_ideal * acceleration_factor;

        // Расчет вектора скорости с добавлением небольшого случайного разброса
        const double noise = 1.0 + (pos_dist(gen) - 0.5) * 0.2; // +/- 10%
        const double vx = -orbital_speed * std::sin(angle) * noise;
        const double vy = orbital_speed * std::cos(angle) * noise;
        
        stars.emplace_back(x, y, vx, vy, mass, STAR_COLORS[color_dist(gen)]);
    }
    
    return stars;
}

// Быстрая отрисовка звезд через VertexArray
void drawStars(sf::RenderWindow& window, const std::vector<Star>& stars, float scale) {
    sf::VertexArray vertices(sf::Points, stars.size());
    
    for (size_t i = 0; i < stars.size(); ++i) {
        const auto& star = stars[i];
        vertices[i].position = sf::Vector2f(static_cast<float>(star.x * scale), 
                                           static_cast<float>(star.y * scale));
        
        if (i == 0) {
            // Отрисовка ЧД с особой подсветкой
            sf::CircleShape shape(8.0f); 
            shape.setFillColor(sf::Color::White);
            shape.setOutlineThickness(2.0f);
            shape.setOrigin(7.0f, 7.0f);
            shape.setPosition(vertices[i].position);
            window.draw(shape);
            
            vertices[i].color = sf::Color::Transparent;
        } else {
            vertices[i].color = star.color;
        }
    }
    
    window.draw(vertices); 
}

// Функция для сохранения результатов бенчмарка
void saveBenchmark(long long total_duration_ms, int num_steps) {
    std::ofstream file(BENCHMARK_FILE, std::ios_base::app);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл бенчмарка: " << BENCHMARK_FILE << std::endl;
        return;
    }

    double avg_time_per_step_ms = (double)total_duration_ms / num_steps;
    double fps_equivalent = 1000.0 / avg_time_per_step_ms;

    file << "--- Benchmark Barnes-Hut (" << NUM_STARS << " частиц) ---\n";
    file << "Date: " << __DATE__ << " " << __TIME__ << "\n";
    file << "Threads used: " << NUM_THREADS << "\n";
    file << "Complexity: O(N log N)\n";
    file << "Theta (Accuracy): " << THETA << "\n";
    file << "Integration steps: " << num_steps << "\n";
    file << "Total duration: " << total_duration_ms << " ms\n";
    file << std::fixed << std::setprecision(3);
    file << "Average time per step: " << avg_time_per_step_ms << " ms\n";
    file << "Equivalent FPS: " << fps_equivalent << "\n";
    file << "------------------------------------------\n";

    std::cout << "\n✅ Результаты бенчмарка (BH) сохранены в файл: " << BENCHMARK_FILE << std::endl;
    std::cout << "   Среднее время на шаг: " << avg_time_per_step_ms << " ms" << std::endl;
    std::cout << "   Эквивалент FPS: " << fps_equivalent << std::endl;
}


int main() {
    
    const double GALAXY_RADIUS = 350.0;
    const double AREA_SIZE = 800.0;
    const int WINDOW_SIZE = 800;
    const float SCALE = WINDOW_SIZE / AREA_SIZE;
    
    std::cout << "🚀 Запуск симуляции N-body с интегрированной ЧД (Barnes-Hut, МАКСИМАЛЬНАЯ ТОЧНОСТЬ)" << std::endl;
    std::cout << "🛠️ DT=" << DT << ", THETA=" << THETA << ", EPSILON_SQ=" << EPSILON_SQ << std::endl;
    std::cout << "Внимание: Эта версия более требовательна к CPU из-за низкого THETA." << std::endl;
    std::cout << "Управление: Space - пауза, T - дерево, ESC - выход" << std::endl;
    
    // Создание окна
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), 
                           "N-Body Galaxy Simulator (Barnes-Hut) - V7 (Max Accuracy + Benchmark)");
    window.setFramerateLimit(60); 
    
    // Создание галактики
    auto stars = createGalaxy(GALAXY_RADIUS, AREA_SIZE/2, AREA_SIZE/2);
    GalaxySimulator simulator(AREA_SIZE, AREA_SIZE);
    
    bool is_paused = false;
    sf::Clock render_clock;
    int step_count = 0;
    
    // --- НАЧАЛО БЕНЧМАРКА ---
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Главный цикл
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Space:
                        is_paused = !is_paused;
                        break;
                    case sf::Keyboard::T:
                        simulator.toggleTree();
                        break;
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    default:
                        break;
                }
            }
        }
        
        // Обновление симуляции
        if (!is_paused && step_count < NUM_STEPS_BENCHMARK) {
            simulator.timeStep(stars);
            step_count++;
        }
        
        // --- КОНЕЦ БЕНЧМАРКА ---
        if (step_count == NUM_STEPS_BENCHMARK) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            // Сохранение и вывод бенчмарка
            saveBenchmark(duration.count(), NUM_STEPS_BENCHMARK);
            
            // Сбрасываем счетчик, чтобы симуляция продолжалась без учета времени
            step_count++; 
            is_paused = true; // Ставим на паузу после бенчмарка для просмотра
            std::cout << "\nСимуляция приостановлена после бенчмарка. Нажмите Space для продолжения.\n";
        }
        
        // Отрисовка
        window.clear(BACKGROUND_COLOR);
        
        // Отрисовка дерева
        simulator.drawTree(window, SCALE);
        
        // Отрисовка всех объектов
        drawStars(window, stars, SCALE);
        
        window.display();
    }
    
    return 0;
}
