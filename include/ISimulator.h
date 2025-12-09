#ifndef ISIMULATOR_H
#define ISIMULATOR_H

#include "Star.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

// Интерфейс для симуляторов
class ISimulator {
public:
    virtual ~ISimulator() = default;
    
    // Выполнить один шаг симуляции
    virtual void timeStep(vector<Star>& stars) = 0;
    
    // Отрисовка дополнительных элементов (например, дерева)
    virtual void draw(RenderWindow& window, float scale) const {}
    
    // Переключение визуализации
    virtual void toggleVisualization() {}
    
    // Получить имя алгоритма
    virtual string getName() const = 0;
    
    // Получить сложность алгоритма
    virtual string getComplexity() const = 0;
};

#endif // ISIMULATOR_H
