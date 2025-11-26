#ifndef ISIMULATOR_H
#define ISIMULATOR_H

#include "Star.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

// Интерфейс для симуляторов
class ISimulator {
public:
    virtual ~ISimulator() = default;
    
    // Выполнить один шаг симуляции
    virtual void timeStep(std::vector<Star>& stars) = 0;
    
    // Отрисовка дополнительных элементов (например, дерева)
    virtual void draw(sf::RenderWindow& window, float scale) const {}
    
    // Переключение визуализации
    virtual void toggleVisualization() {}
    
    // Получить имя алгоритма
    virtual std::string getName() const = 0;
    
    // Получить сложность алгоритма
    virtual std::string getComplexity() const = 0;
};

#endif // ISIMULATOR_H
