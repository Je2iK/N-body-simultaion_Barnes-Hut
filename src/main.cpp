#include "BarnesHutSimulator.h"
#include "BruteForceSimulator.h"
#include "Benchmark.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <SFML/Graphics.hpp>
#include "Menu.h"
using namespace std;
using namespace sf;
bool runVisualization(ISimulator* simulator, vector<Star> stars, const string& window_title) {
    const int WINDOW_WIDTH = 1600;
    const int WINDOW_HEIGHT = 1600;
    const float SCALE = WINDOW_WIDTH / 1600.0;
    RenderWindow window(VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), window_title);
    window.setFramerateLimit(60);
    View view = window.getDefaultView();
    bool isDragging = false;
    Vector2i lastMousePos;
    float currentZoom = 1.0f;
    int selectedStarIndex = -1;
    bool showTrails = false;
    bool show_help = false;
    int trailUpdateCounter = 0;
    const int TRAIL_UPDATE_FREQ = 5;
    const int MAX_TRAIL_LENGTH = 5;
    Font font;
    if (!loadFont(font)) {
        cerr << "Failed to load any font!" << endl;
    }
    bool is_paused = false;
    int step_count = 0;
    int fps_display = 0;
    Clock fps_clock;
    bool isBruteForce = (simulator->getName().find("Brute Force") != string::npos);
    String algoName = isBruteForce ? ru(u8"Полный перебор") : ru(u8"Барнс-Хат");
    String particlesStr = ru(u8"Частицы");
    String fpsStr = ru("FPS");
    String thetaStr = ru(u8"Тета");
    String cellStr = ru(u8"Ячейка");
    String helpPromptStr = ru(u8"H - Управление");
    String selectedStr = ru(u8"ВЫБРАНО");
    string massLabel = u8"Масса: ";
    string posLabel = u8"Позиция: ";
    string velLabel = u8"Скорость: ";
    String controlsTitle = ru(u8"УПРАВЛЕНИЕ");
    stringstream helpSS_pre;
    helpSS_pre << u8"SPACE       Пауза/Старт" << "\n";
    helpSS_pre << u8"E           Следы вкл/выкл" << "\n";
    helpSS_pre << u8"R           Центр на черной дыре" << "\n";
    helpSS_pre << u8"WASD/Arrows Перемещение" << "\n";
    helpSS_pre << u8"Колесо мыши Зум" << "\n";
    helpSS_pre << u8"ЛКМ         Выбрать звезду" << "\n";
    if (!isBruteForce) {
        helpSS_pre << "\n";
        helpSS_pre << u8"T           Сетка вкл/выкл" << "\n";
        helpSS_pre << u8"+/-         Изменить Тета" << "\n";
        helpSS_pre << u8"[/]         Размер ячейки" << "\n";
    }
    helpSS_pre << "\n";
    helpSS_pre << u8"H - Показать/Скрыть управление" << "\n";
    helpSS_pre << u8"ESC - Вернуться в меню";
    string helpStringUtf8 = helpSS_pre.str();
    String helpStringFinal = String::fromUtf8(helpStringUtf8.begin(), helpStringUtf8.end());
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return true; 
            }
            if (const auto* scroll = event->getIf<Event::MouseWheelScrolled>()) {
                if (scroll->delta > 0) {
                    view.zoom(0.9f);
                    currentZoom *= 0.9f;
                } else {
                    view.zoom(1.1f);
                    currentZoom *= 1.1f;
                }
            }
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Middle || mouseBtn->button == Mouse::Button::Right) {
                    isDragging = true;
                    lastMousePos = Mouse::getPosition(window);
                } else if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f worldPos = window.mapPixelToCoords(Mouse::getPosition(window), view);
                    double minDist = 20.0 * currentZoom;
                    int closest = -1;
                    for (size_t i = 0; i < stars.size(); ++i) {
                        double dx = stars[i].x * SCALE - worldPos.x;
                        double dy = stars[i].y * SCALE - worldPos.y;
                        double dist = sqrt(dx*dx + dy*dy);
                        if (dist < minDist) {
                            minDist = dist;
                            closest = static_cast<int>(i);
                        }
                    }
                    selectedStarIndex = closest;
                }
            }
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonReleased>()) {
                if (mouseBtn->button == Mouse::Button::Middle || mouseBtn->button == Mouse::Button::Right) {
                    isDragging = false;
                }
            }
            if (event->is<Event::MouseMoved>()) {
                if (isDragging) {
                    Vector2i currentMousePos = Mouse::getPosition(window);
                    Vector2f delta = window.mapPixelToCoords(lastMousePos, view) - 
                                       window.mapPixelToCoords(currentMousePos, view);
                    view.move(delta);
                    lastMousePos = currentMousePos;
                }
            }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                switch (key->code) {
                    case Keyboard::Key::Space:
                        is_paused = !is_paused;
                        break;
                    case Keyboard::Key::T:
                        if (!isBruteForce) simulator->toggleVisualization();
                        break;
                    case Keyboard::Key::E:
                        showTrails = !showTrails;
                        if (!showTrails) {
                            for (auto& star : stars) star.trail.clear();
                        }
                        break;
                    case Keyboard::Key::R:
                        if (!stars.empty()) {
                            view.setCenter({static_cast<float>(stars[0].x * SCALE), 
                                          static_cast<float>(stars[0].y * SCALE)});
                            view.setSize(window.getDefaultView().getSize());
                            currentZoom = 1.0f;
                        }
                        break;
                    case Keyboard::Key::Equal:
                    case Keyboard::Key::Hyphen:
                        if (!isBruteForce) {
                            auto* bhSim = dynamic_cast<BarnesHutSimulator*>(simulator);
                            if (bhSim) {
                                if (key->code == Keyboard::Key::Equal) {
                                    bhSim->increaseTheta();
                                } else {
                                    bhSim->decreaseTheta();
                                }
                            }
                        }
                        break;
                    case Keyboard::Key::LBracket:
                    case Keyboard::Key::RBracket:
                        if (!isBruteForce) {
                            auto* bhSim = dynamic_cast<BarnesHutSimulator*>(simulator);
                            if (bhSim) {
                                if (key->code == Keyboard::Key::RBracket) {
                                    bhSim->increaseMinCellSize();
                                } else {
                                    bhSim->decreaseMinCellSize();
                                }
                            }
                        }
                        break;
                    case Keyboard::Key::H:
                        show_help = !show_help;
                        break;
                    case Keyboard::Key::Escape:
                        window.close();
                        return false;
                        break;
                    default:
                        break;
                }
            }
        }
        float moveSpeed = 15.0f * currentZoom;
        if (Keyboard::isKeyPressed(Keyboard::Key::W) || Keyboard::isKeyPressed(Keyboard::Key::Up)) view.move({0, -moveSpeed});
        if (Keyboard::isKeyPressed(Keyboard::Key::S) || Keyboard::isKeyPressed(Keyboard::Key::Down)) view.move({0, moveSpeed});
        if (Keyboard::isKeyPressed(Keyboard::Key::A) || Keyboard::isKeyPressed(Keyboard::Key::Left)) view.move({-moveSpeed, 0});
        if (Keyboard::isKeyPressed(Keyboard::Key::D) || Keyboard::isKeyPressed(Keyboard::Key::Right)) view.move({moveSpeed, 0});
        if (!is_paused) {
            simulator->timeStep(stars);
            step_count++;
            if (showTrails) {
                trailUpdateCounter++;
                if (trailUpdateCounter >= TRAIL_UPDATE_FREQ) {
                    trailUpdateCounter = 0;
                    for (auto& star : stars) {
                        star.trail.push_back(Vector2f(star.x, star.y));
                        if (star.trail.size() > MAX_TRAIL_LENGTH) {
                            star.trail.pop_front();
                        }
                    }
                }
            }
            if (fps_clock.getElapsedTime().asSeconds() >= 1.0) {
                fps_display = step_count;
                step_count = 0;
                fps_clock.restart();
            }
        }
        window.clear(Visual::BACKGROUND_COLOR);
        window.setView(view);
        simulator->draw(window, SCALE);
        drawStars(window, stars, SCALE, showTrails);
        if (selectedStarIndex >= 0 && selectedStarIndex < static_cast<int>(stars.size())) {
            const auto& star = stars[selectedStarIndex];
            CircleShape marker(15.0f * currentZoom);
            marker.setOrigin({15.0f * currentZoom, 15.0f * currentZoom});
            marker.setPosition({static_cast<float>(star.x * SCALE), static_cast<float>(star.y * SCALE)});
            marker.setFillColor(Color::Transparent);
            marker.setOutlineColor(Color(30, 215, 96));
            marker.setOutlineThickness(2.0f * currentZoom);
            window.draw(marker);
        }
        window.setView(window.getDefaultView());
        RectangleShape topBar({static_cast<float>(WINDOW_WIDTH), 50});
        topBar.setPosition({0, 0});
        topBar.setFillColor(Color(18, 18, 18, 200));
        window.draw(topBar);
        CircleShape statusDot(8);
        statusDot.setPosition({15, 19});
        if (is_paused) {
            statusDot.setFillColor(Color(255, 100, 100));
        } else {
            statusDot.setFillColor(Color(30, 215, 96));
        }
        window.draw(statusDot);
        Text algoText(font);
        algoText.setString(algoName);
        algoText.setCharacterSize(14);
        algoText.setFillColor(Color::White);
        algoText.setStyle(Text::Bold);
        algoText.setPosition({35, 16});
        window.draw(algoText);
        RectangleShape divider1({2, 30});
        divider1.setPosition({150, 10});
        divider1.setFillColor(Color(60, 60, 60));
        window.draw(divider1);
        float xPos = 165;
        Text particlesLabel(font);
        particlesLabel.setString(particlesStr);
        particlesLabel.setCharacterSize(10);
        particlesLabel.setFillColor(Color(120, 120, 120));
        particlesLabel.setPosition({xPos, 12});
        window.draw(particlesLabel);
        Text particlesValue(font);
        particlesValue.setString(to_string(stars.size()));
        particlesValue.setCharacterSize(14);
        particlesValue.setFillColor(Color::White);
        particlesValue.setStyle(Text::Bold);
        particlesValue.setPosition({xPos, 26});
        window.draw(particlesValue);
        xPos += 100;
        Text fpsLabel(font);
        fpsLabel.setString(fpsStr);
        fpsLabel.setCharacterSize(10);
        fpsLabel.setFillColor(Color(120, 120, 120));
        fpsLabel.setPosition({xPos, 12});
        window.draw(fpsLabel);
        Text fpsValue(font);
        fpsValue.setString(to_string(fps_display));
        fpsValue.setCharacterSize(14);
        fpsValue.setFillColor(Color(30, 215, 96));
        fpsValue.setStyle(Text::Bold);
        fpsValue.setPosition({xPos, 26});
        window.draw(fpsValue);
        if (!isBruteForce) {
            auto* bhSim = dynamic_cast<BarnesHutSimulator*>(simulator);
            if (bhSim) {
                xPos += 80;
                RectangleShape divider2({2, 30});
                divider2.setPosition({xPos - 10, 10});
                divider2.setFillColor(Color(60, 60, 60));
                window.draw(divider2);
                Text thetaLabel(font);
                thetaLabel.setString(thetaStr);
                thetaLabel.setCharacterSize(10);
                thetaLabel.setFillColor(Color(120, 120, 120));
                thetaLabel.setPosition({xPos, 12});
                window.draw(thetaLabel);
                stringstream thetaSS;
                thetaSS << fixed << setprecision(1) << bhSim->getTheta();
                Text thetaValue(font);
                thetaValue.setString(thetaSS.str());
                thetaValue.setCharacterSize(14);
                thetaValue.setFillColor(Color(100, 150, 255));
                thetaValue.setStyle(Text::Bold);
                thetaValue.setPosition({xPos, 26});
                window.draw(thetaValue);
                xPos += 70;
                Text cellLabel(font);
                cellLabel.setString(cellStr);
                cellLabel.setCharacterSize(10);
                cellLabel.setFillColor(Color(120, 120, 120));
                cellLabel.setPosition({xPos, 12});
                window.draw(cellLabel);
                stringstream cellSS;
                cellSS << fixed << setprecision(0) << bhSim->getMinCellSize();
                Text cellValue(font);
                cellValue.setString(cellSS.str());
                cellValue.setCharacterSize(14);
                cellValue.setFillColor(Color(255, 150, 100));
                cellValue.setStyle(Text::Bold);
                cellValue.setPosition({xPos, 26});
                window.draw(cellValue);
            }
        }
        Text helpText(font);
        helpText.setString(helpPromptStr);
        helpText.setCharacterSize(11);
        helpText.setFillColor(Color(100, 100, 100));
        FloatRect helpBounds = helpText.getLocalBounds();
        helpText.setPosition({static_cast<float>(WINDOW_WIDTH) - helpBounds.size.x - 20, 18});
        window.draw(helpText);
        if (selectedStarIndex >= 0 && selectedStarIndex < static_cast<int>(stars.size())) {
            const auto& star = stars[selectedStarIndex];
            RectangleShape starPanel({250, 70});
            starPanel.setPosition({15, static_cast<float>(WINDOW_HEIGHT) - 85});
            starPanel.setFillColor(Color(18, 18, 18, 200));
            window.draw(starPanel);
            Text starTitle(font);
            starTitle.setString(selectedStr);
            starTitle.setCharacterSize(10);
            starTitle.setFillColor(Color(30, 215, 96));
            starTitle.setStyle(Text::Bold);
            starTitle.setPosition({25, static_cast<float>(WINDOW_HEIGHT) - 77});
            window.draw(starTitle);
            Text starInfo(font);
            stringstream starSS;
            starSS << massLabel << fixed << setprecision(1) << star.mass << "\n";
            starSS << posLabel << "(" << (int)star.x << ", " << (int)star.y << ")\n";
            double vel = sqrt(star.vx*star.vx + star.vy*star.vy);
            starSS << velLabel << fixed << setprecision(2) << vel;
            string s = starSS.str();
            starInfo.setString(String::fromUtf8(s.begin(), s.end()));
            starInfo.setCharacterSize(11);
            starInfo.setFillColor(Color(180, 180, 180));
            starInfo.setPosition({25, static_cast<float>(WINDOW_HEIGHT) - 62});
            window.draw(starInfo);
        }
        if (show_help) {
            RectangleShape helpBg({400, isBruteForce ? 280.0f : 340.0f});
            helpBg.setPosition({static_cast<float>(WINDOW_WIDTH)/2 - 200, static_cast<float>(WINDOW_HEIGHT)/2 - (isBruteForce ? 140.0f : 170.0f)});
            helpBg.setFillColor(Color(18, 18, 18, 240));
            helpBg.setOutlineColor(Color(30, 215, 96));
            helpBg.setOutlineThickness(2);
            window.draw(helpBg);
            float helpY = static_cast<float>(WINDOW_HEIGHT)/2 - (isBruteForce ? 120.0f : 150.0f);
            Text helpTitle(font);
            helpTitle.setString(controlsTitle);
            helpTitle.setCharacterSize(16);
            helpTitle.setFillColor(Color(30, 215, 96));
            helpTitle.setStyle(Text::Bold);
            FloatRect titleBounds = helpTitle.getLocalBounds();
            helpTitle.setPosition({static_cast<float>(WINDOW_WIDTH)/2 - titleBounds.size.x/2, helpY});
            window.draw(helpTitle);
            helpY += 35;
            Text helpControls(font);
            helpControls.setString(helpStringFinal);
            helpControls.setCharacterSize(13);
            helpControls.setFillColor(Color(200, 200, 200));
            helpControls.setPosition({static_cast<float>(WINDOW_WIDTH)/2 - 170, helpY});
            window.draw(helpControls);
        }
        window.display();
    }
    return false;
}
bool runBenchmarkGUI(ISimulator* sim1, int particleCount, ISimulator* sim2 = nullptr) {
    RenderWindow window(VideoMode({900, 700}), ru(u8"РЕЗУЛЬТАТЫ ТЕСТА"), Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    Font font;
    if (!loadFont(font)) {
        cerr << "Failed to load any font!" << endl;
    }
    string resultStr = u8"Запуск теста...\nПожалуйста, подождите.";
    atomic<bool> isDone{false};
    thread benchThread([&]() {
        if (sim2) {
            resultStr = Benchmark::getComparisonResult(sim1, sim2, 
                                                     particleCount, 
                                                     Simulation::NUM_STEPS_BENCHMARK);
        } else {
            auto stars = createGalaxy(particleCount, 500.0, 1600.0, 1600.0);
            auto res = Benchmark::run(sim1, stars, Simulation::NUM_STEPS_BENCHMARK);
            bool isBrute = (sim1->getName().find("Brute") != string::npos);
            string algoRu = isBrute ? u8"Полный перебор" : u8"Барнс-Хат";
            stringstream ss;
            ss << u8"РЕЗУЛЬТАТЫ ТЕСТА" << "\n\n";
            ss << u8"Алгоритм: " << algoRu << "\n";
            ss << u8"Частицы: " << res.num_particles << "\n";
            ss << u8"Шаги: " << res.num_steps << "\n";
            ss << u8"Время: " << res.total_duration_ms << " ms\n";
            ss << "FPS: " << fixed << setprecision(1) << res.fps_equivalent << "\n";
            resultStr = ss.str();
            Benchmark::saveResult(res);
        }
        resultStr += u8"\n\nНажмите SPACE или ESC для возврата.";
        isDone = true;
    });
    float spinnerAngle = 0;
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                if (benchThread.joinable()) benchThread.join();
                return true;
            }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (isDone && (key->code == Keyboard::Key::Space || key->code == Keyboard::Key::Escape)) {
                    window.close();
                }
            }
        }
        window.clear(Color(18, 18, 18));
        Text title(font);
        title.setFont(font);
        title.setString(ru(u8"БЕНЧМАРК"));
        title.setCharacterSize(42);
        title.setFillColor(Color::White);
        title.setStyle(Text::Bold);
        FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin({titleBounds.position.x + titleBounds.size.x/2.0f, titleBounds.position.y + titleBounds.size.y/2.0f});
        title.setPosition({450, 50});
        window.draw(title);
        RectangleShape accentLine(Vector2f(80, 4));
        accentLine.setPosition({410, 85});
        accentLine.setFillColor(Color(30, 215, 96));
        window.draw(accentLine);
        RectangleShape panel(Vector2f(850, 500));
        panel.setPosition({25, 120});
        panel.setFillColor(Color(24, 24, 24, 255));
        panel.setOutlineThickness(0);
        window.draw(panel);
        RectangleShape leftBar(Vector2f(4, 500));
        leftBar.setPosition({25, 120});
        leftBar.setFillColor(Color(30, 215, 96));
        window.draw(leftBar);
        Text text(font);
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color(179, 179, 179));
        text.setPosition({50, 150});
        text.setString(String::fromUtf8(resultStr.begin(), resultStr.end()));
        window.draw(text);
        if (!isDone) {
            spinnerAngle += 5.0f;
            for (int i = 0; i < 8; ++i) {
                float angle = spinnerAngle + i * 45.0f;
                float rad = angle * 3.14159f / 180.0f;
                float x = 450 + cos(rad) * 60;
                float y = 400 + sin(rad) * 60;
                CircleShape dot(6);
                dot.setOrigin({6, 6});
                dot.setPosition({x, y});
                int alpha = 255 - (i * 30);
                dot.setFillColor(Color(30, 215, 96, alpha));
                window.draw(dot);
            }
            Text loadingText(font);
            loadingText.setFont(font);
            loadingText.setString(ru(u8"Обработка..."));
            loadingText.setCharacterSize(20);
            loadingText.setFillColor(Color(179, 179, 179));
            FloatRect loadBounds = loadingText.getLocalBounds();
            loadingText.setOrigin({loadBounds.position.x + loadBounds.size.x/2.0f, loadBounds.position.y + loadBounds.size.y/2.0f});
            loadingText.setPosition({450, 480});
            window.draw(loadingText);
        }
        window.display();
    }
    if (benchThread.joinable()) {
        benchThread.join();
    }
    return false;
}
int main() {
    const double AREA_SIZE = 1600.0;
    const double GALAXY_RADIUS = 500.0;
    Menu menu(900, 700);
    while (true) {
        #ifdef ENABLE_AUTH
        if (!menu.showLoginScreen()) {
            return 0;
        }
        #endif
        bool loggedIn = true;
        while (loggedIn) {
            int choice = menu.run();
            if (choice == 0) {
                return 0;
            }
            if (choice == -1) {
                loggedIn = false;
                continue;
            }
            int particleCount = 1000;
            if (choice >= 1 && choice <= 6) {
                particleCount = menu.selectParticleCount();
                if (particleCount == 0) return 0;
            }
            switch (choice) {
                case 1: {
                    auto simulator = make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                    auto stars = createGalaxy(particleCount, GALAXY_RADIUS, AREA_SIZE/2, AREA_SIZE/2, 1.0, 0.0, 0.0);
                    if (runVisualization(simulator.get(), move(stars), "N-Body Simulation - Barnes-Hut")) return 0;
                    break;
                }
                case 2: {
                    auto simulator = make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                    auto stars = createGalaxy(particleCount, GALAXY_RADIUS, AREA_SIZE/2, AREA_SIZE/2, 1.0, 0.0, 0.0);
                    if (runVisualization(simulator.get(), move(stars), "N-Body Simulation - Brute Force")) return 0;
                    break;
                }
                case 3: {
                    auto simulator = make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                    if (runBenchmarkGUI(simulator.get(), particleCount)) return 0;
                    break;
                }
                case 4: {
                    auto simulator = make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                    if (runBenchmarkGUI(simulator.get(), particleCount)) return 0;
                    break;
                }
                case 5: {
                    auto sim1 = make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                    auto sim2 = make_unique<BruteForceSimulator>(AREA_SIZE, AREA_SIZE);
                    if (runBenchmarkGUI(sim1.get(), particleCount, sim2.get())) return 0;
                    break;
                }
                case 6: {
                    auto simulator = make_unique<BarnesHutSimulator>(AREA_SIZE, AREA_SIZE);
                    auto stars = createGalaxyCollision(particleCount, GALAXY_RADIUS * 0.6, AREA_SIZE, AREA_SIZE);
                    if (runVisualization(simulator.get(), move(stars), "N-Body Simulation - Galaxy Collision")) return 0;
                    break;
                }
                default:
                    break;
            }
        }
    }
    return 0;
}
