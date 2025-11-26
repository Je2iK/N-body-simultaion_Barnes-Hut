#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

class Menu {
public:
    Menu(float width, float height);
    
    // Возвращает выбранный пункт меню (или -1 если выход)
    int run();

private:
    void draw(sf::RenderWindow& window);
    void handleInput(sf::RenderWindow& window);
    
    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        int id;
        float currentScale = 1.0f;     // Для анимации масштаба
        float targetScale = 1.0f;
        float alpha = 150.0f;          // Прозрачность фона
    };
    
    struct BackgroundStar {
        float x, y;
        float speed;
        float size;
        float brightness;
    };
    
    float width;
    float height;
    sf::Font font;
    std::vector<Button> buttons;
    std::vector<BackgroundStar> stars; // Фоновые звезды
    
    void addButton(const std::string& text, int id, float y);
    void updateBackground();           // Обновление звезд
    void drawBackground(sf::RenderWindow& window);
};

#endif // MENU_H
