#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

using namespace std;
using namespace sf;

class Menu {
public:
    Menu(float width, float height);
    
    // Returns selected menu item (or 0 if exit)
    int run();
    
    // Returns selected particle count (100, 500, 1000, 2500, 5000)
    int selectParticleCount();
    
    // Returns selected theta value (0.1 - 2.0)
    double selectTheta();
    
    // Auth methods
    bool showLoginScreen();
    bool showRegisterScreen();

private:
    void draw(RenderWindow& window);
    void handleInput(RenderWindow& window);
    
    struct Button {
        RectangleShape shape;
        Text text;
        int id;
        float currentScale = 1.0f;
        float targetScale = 1.0f;
        float alpha = 150.0f;
        string locKey;

        Button(const Font& font) : text(font) {}
    };
    
    struct InputBox {
        RectangleShape shape;
        Text text;
        Text label;
        string value;
        bool isActive = false;
        bool isPassword = false;

        InputBox(const Font& font) : text(font), label(font) {}
        
        void handleInput(const Event& event) {
            if (!isActive) return;
            
            if (const auto* textEvent = event.getIf<Event::TextEntered>()) {
                if (textEvent->unicode == 8) { // Backspace
                    if (!value.empty()) value.pop_back();
                } else if (textEvent->unicode < 128 && textEvent->unicode > 31) {
                    if (value.length() < 20) {
                        value += static_cast<char>(textEvent->unicode);
                    }
                }
            }
        }
        
        void updateDisplay() {
            if (isPassword) {
                string masked(value.length(), '*');
                text.setString(masked + (isActive ? "|" : ""));
            } else {
                text.setString(value + (isActive ? "|" : ""));
            }
        }
    };
    
    struct BackgroundStar {
        float x, y;
        float speed;
        float size;
        float brightness;
    };
    
    float width;
    float height;
    Font font;
    vector<Button> buttons;
    vector<BackgroundStar> stars;
    string current_username;  // Store logged in username
    
    void addButton(const string& text, int id, float x, float y, float width, float height);
    void updateBackground();
    void drawBackground(RenderWindow& window);
    void showAdminPanel();
    void showSettings();
};

#endif // MENU_H
