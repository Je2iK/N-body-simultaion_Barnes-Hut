#include "BarnesHutSimulator.h"
#include "BruteForceSimulator.h"
#include "Benchmark.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

#include "Menu.h"

// Прототипы или реализации функций
void runVisualization(ISimulator* simulator, std::vector<Star> stars, const std::string& window_title) {
    const int WINDOW_SIZE = 1600;
    const float SCALE = WINDOW_SIZE / 1600.0; // AREA_SIZE is 1600
    
    std::cout << "\n🚀 Starting " << simulator->getName() << " simulation\n";
    std::cout << "   Particles: " << stars.size() << "\n";
    std::cout << "   Threads: " << Simulation::NUM_THREADS << "\n";
    std::cout << "   Complexity: " << simulator->getComplexity() << "\n\n";
    std::cout << "Controls:\n";
    std::cout << "  SPACE - Pause/Resume\n";
    std::cout << "  T     - Toggle tree visualization (Barnes-Hut only)\n";
    std::cout << "  ESC   - Exit\n\n";
    
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), window_title);
    window.setFramerateLimit(60);
    
    bool is_paused = false;
    int step_count = 0;
    sf::Clock fps_clock;
    
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
                        std::cout << (is_paused ? "⏸ Paused\n" : "▶ Resumed\n");
                        break;
                    case sf::Keyboard::T:
                        simulator->toggleVisualization();
                        std::cout << "🌳 Tree visualization toggled\n";
                        break;
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    default:
                        break;
                }
            }
        }
        
        // Update simulation
        if (!is_paused) {
            simulator->timeStep(stars);
            step_count++;
            
            // Print FPS every second
            if (fps_clock.getElapsedTime().asSeconds() >= 1.0) {
                std::cout << "FPS: " << step_count << " | Steps: " << step_count << "\r" << std::flush;
                step_count = 0;
                fps_clock.restart();
            }
        }
        
        // Render
        window.clear(Visual::BACKGROUND_COLOR);
        simulator->draw(window, SCALE);
        drawStars(window, stars, SCALE);
        window.display();
    }
}

#include <thread>
#include <atomic>
#include <sstream>
#include <iomanip>

// ... (runVisualization remains the same) ...

void runBenchmarkGUI(ISimulator* sim1, ISimulator* sim2 = nullptr) {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Benchmark Results");
    window.setFramerateLimit(60);
    
    sf::Font font;
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "arial.ttf"
    };
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) break;
    }
    
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition(50, 50);
    
    std::string resultStr = "Running benchmark...\nPlease wait, this may take a while.";
    std::atomic<bool> isDone{false};
    
    // Запускаем бенчмарк в отдельном потоке
    std::thread benchThread([&]() {
        if (sim2) {
            // Сравнение
            resultStr = Benchmark::getComparisonResult(sim1, sim2, 
                                                     Simulation::NUM_STARS, 
                                                     Simulation::NUM_STEPS_BENCHMARK);
        } else {
            // Одиночный бенчмарк
            auto stars = createGalaxy(500.0, 1600.0, 1600.0);
            auto res = Benchmark::run(sim1, stars, Simulation::NUM_STEPS_BENCHMARK);
            
            std::stringstream ss;
            ss << "BENCHMARK RESULTS\n\n";
            ss << "Algorithm: " << sim1->getName() << "\n";
            ss << "Particles: " << res.num_particles << "\n";
            ss << "Steps:     " << res.num_steps << "\n";
            ss << "Time:      " << res.total_duration_ms << " ms\n";
            ss << "FPS:       " << std::fixed << std::setprecision(1) << res.fps_equivalent << "\n";
            resultStr = ss.str();
        }
        resultStr += "\n\nPress SPACE or ESC to return.";
        isDone = true;
    });
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (isDone && (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Escape)) {
                    window.close();
                }
            }
        }
        
        window.clear(sf::Color(20, 20, 30));
        
        text.setString(resultStr);
        window.draw(text);
        
        // Анимация загрузки
        if (!isDone) {
            static float angle = 0;
            angle += 5.0f;
            sf::RectangleShape spinner(sf::Vector2f(50, 50));
            spinner.setOrigin(25, 25);
            spinner.setPosition(400, 400);
            spinner.setFillColor(sf::Color::Cyan);
            spinner.setRotation(angle);
            window.draw(spinner);
        }
        
        window.display();
    }
    
    if (benchThread.joinable()) {
        benchThread.join();
    }
}

int main() {
    const double AREA_SIZE = 1600.0;
    const double GALAXY_RADIUS = 500.0;
    
    Menu menu(800, 800);
    
    while (true) {
        int choice = menu.run();
        
        if (choice == 0) {
            break;
        }
        
        switch (choice) {
            case 1: {
                auto simulator = std::make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                auto stars = createGalaxy(GALAXY_RADIUS, AREA_SIZE/2, AREA_SIZE/2, 0.99, 0.2, 0.1);
                runVisualization(simulator.get(), std::move(stars), "N-Body Simulation - Barnes-Hut");
                break;
            }
            case 2: {
                auto simulator = std::make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                auto stars = createGalaxy(GALAXY_RADIUS, AREA_SIZE/2, AREA_SIZE/2, 0.99, 0.2, 0.1);
                runVisualization(simulator.get(), std::move(stars), "N-Body Simulation - Brute Force");
                break;
            }
            case 3: {
                auto simulator = std::make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                runBenchmarkGUI(simulator.get());
                break;
            }
            case 4: {
                auto simulator = std::make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                runBenchmarkGUI(simulator.get());
                break;
            }
            case 5: {
                auto sim1 = std::make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                auto sim2 = std::make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                runBenchmarkGUI(sim1.get(), sim2.get());
                break;
            }
            case 6: {
                auto simulator = std::make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                auto stars = createGalaxyCollision(GALAXY_RADIUS * 0.6, AREA_SIZE, AREA_SIZE);
                runVisualization(simulator.get(), std::move(stars), "N-Body Simulation - Galaxy Collision");
                break;
            }
            default:
                break;
        }
    }
    
    return 0;
}
