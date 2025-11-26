#include "Menu.h"
#include <iostream>
#include <cmath>
#include <random>

// Линейная интерполяция для плавной анимации
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

Menu::Menu(float width, float height) : width(width), height(height) {
    // 1. Загрузка шрифта
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "arial.ttf"
    };

    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load any font.\n";
    }

    // 2. Инициализация фоновых звезд
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> x_dist(0, width);
    std::uniform_real_distribution<> y_dist(0, height);
    std::uniform_real_distribution<> speed_dist(0.2, 1.5);
    std::uniform_real_distribution<> size_dist(1.0, 3.0);
    std::uniform_real_distribution<> bright_dist(100, 255);

    for (int i = 0; i < 200; ++i) {
        stars.push_back({
            (float)x_dist(gen),
            (float)y_dist(gen),
            (float)speed_dist(gen),
            (float)size_dist(gen),
            (float)bright_dist(gen)
        });
    }

    // 3. Создание кнопок
    float startY = 200.0f;
    float gap = 80.0f; // Больше расстояние

    addButton("Barnes-Hut Simulation", 1, startY);
    addButton("Brute Force Simulation", 2, startY + gap);
    addButton("Benchmark: Barnes-Hut", 3, startY + gap * 2);
    addButton("Benchmark: Brute Force", 4, startY + gap * 3);
    addButton("Compare Algorithms", 5, startY + gap * 4);
    addButton("Galaxy Collision", 6, startY + gap * 5);
    addButton("Exit", 0, startY + gap * 6.2f);
}

void Menu::addButton(const std::string& textStr, int id, float y) {
    Button btn;
    btn.id = id;
    
    float btnWidth = 500.0f;
    float btnHeight = 60.0f;
    
    // Форма кнопки
    btn.shape.setSize(sf::Vector2f(btnWidth, btnHeight));
    btn.shape.setOrigin(btnWidth / 2, btnHeight / 2);
    btn.shape.setPosition(width / 2, y);
    
    // Стиль кнопки (Glassmorphism)
    btn.shape.setFillColor(sf::Color(20, 20, 30, 200)); // Полупрозрачный темный
    btn.shape.setOutlineThickness(2.0f);
    btn.shape.setOutlineColor(sf::Color(60, 60, 80)); // Тусклая рамка по умолчанию
    
    // Текст
    btn.text.setFont(font);
    btn.text.setString(textStr);
    btn.text.setCharacterSize(24);
    btn.text.setFillColor(sf::Color(200, 200, 200)); // Светло-серый текст
    
    sf::FloatRect textRect = btn.text.getLocalBounds();
    btn.text.setOrigin(textRect.left + textRect.width/2.0f,
                       textRect.top  + textRect.height/2.0f);
    btn.text.setPosition(width / 2, y);
    
    buttons.push_back(btn);
}

void Menu::updateBackground() {
    for (auto& star : stars) {
        star.y += star.speed;
        if (star.y > height) {
            star.y = 0;
            star.x = static_cast<float>(rand() % (int)width);
        }
    }
}

void Menu::drawBackground(sf::RenderWindow& window) {
    sf::VertexArray vertices(sf::Points, stars.size());
    for (size_t i = 0; i < stars.size(); ++i) {
        vertices[i].position = sf::Vector2f(stars[i].x, stars[i].y);
        int b = static_cast<int>(stars[i].brightness);
        vertices[i].color = sf::Color(b, b, b, b); // Прозрачность зависит от яркости
    }
    window.draw(vertices);
}

int Menu::run() {
    sf::RenderWindow window(sf::VideoMode(width, height), "N-Body Simulation", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (const auto& btn : buttons) {
                        if (btn.shape.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                            window.close();
                            return btn.id;
                        }
                    }
                }
            }
        }
        
        // Анимация и логика
        updateBackground();
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        float dt = 0.1f; // Фактор плавности

        for (auto& btn : buttons) {
            bool hovered = btn.shape.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            
            if (hovered) {
                btn.targetScale = 1.05f;
                btn.shape.setOutlineColor(sf::Color::Cyan); // Неоновая рамка
                btn.shape.setFillColor(sf::Color(40, 40, 60, 230)); // Чуть светлее
                btn.text.setFillColor(sf::Color::White);
            } else {
                btn.targetScale = 1.0f;
                btn.shape.setOutlineColor(sf::Color(60, 60, 80));
                btn.shape.setFillColor(sf::Color(20, 20, 30, 200));
                btn.text.setFillColor(sf::Color(200, 200, 200));
            }
            
            // Плавное изменение масштаба
            btn.currentScale = lerp(btn.currentScale, btn.targetScale, dt);
            btn.shape.setScale(btn.currentScale, btn.currentScale);
            btn.text.setScale(btn.currentScale, btn.currentScale);
        }

        // Отрисовка
        window.clear(sf::Color(10, 10, 15)); // Очень темный фон (почти черный)
        drawBackground(window);
        
        // Заголовок с тенью
        sf::Text titleShadow;
        titleShadow.setFont(font);
        titleShadow.setString("N-BODY SIMULATION");
        titleShadow.setCharacterSize(50);
        titleShadow.setFillColor(sf::Color(0, 255, 255, 50)); // Тень (Cyan Glow)
        titleShadow.setStyle(sf::Text::Bold);
        
        sf::FloatRect titleRect = titleShadow.getLocalBounds();
        titleShadow.setOrigin(titleRect.left + titleRect.width/2.0f,
                              titleRect.top  + titleRect.height/2.0f);
        titleShadow.setPosition(width / 2 + 4, 84); // Смещение тени
        
        sf::Text title = titleShadow;
        title.setFillColor(sf::Color::White);
        title.setPosition(width / 2, 80);
        
        window.draw(titleShadow);
        window.draw(title);
        
        // Подзаголовок
        sf::Text subtitle;
        subtitle.setFont(font);
        subtitle.setString("High Performance Physics Engine");
        subtitle.setCharacterSize(20);
        subtitle.setFillColor(sf::Color(150, 150, 150));
        sf::FloatRect subRect = subtitle.getLocalBounds();
        subtitle.setOrigin(subRect.left + subRect.width/2.0f, subRect.top + subRect.height/2.0f);
        subtitle.setPosition(width/2, 130);
        window.draw(subtitle);

        // Кнопки
        for (const auto& btn : buttons) {
            window.draw(btn.shape);
            window.draw(btn.text);
        }
        
        window.display();
    }
    return 0;
}
