#include "Cell.h"
#include "Constants.h"
Cell::Cell(double x, double y, double width, double height)
    : x(x), y(y), width(width), height(height),
      mass(0), com_x(0), com_y(0), is_divided(false)
{
    stars.reserve(1);
}
void Cell::draw(sf::RenderWindow& window, float scale) const {
    if (stars.empty() && children.empty()) return;
    if (width * scale < 5.0f && !is_divided) return;
    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(width * scale),
                                        static_cast<float>(height * scale)));
    rect.setPosition({static_cast<float>((x - width/2) * scale),
                     static_cast<float>((y - height/2) * scale)});
    rect.setFillColor(sf::Color::Transparent);
    rect.setOutlineColor(Visual::TREE_COLOR);
    rect.setOutlineThickness(0.3f);
    window.draw(rect);
    for (const auto& child : children) {
        child->draw(window, scale);
    }
}
