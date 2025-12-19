#ifndef ISIMULATOR_H
#define ISIMULATOR_H
#include "Star.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;
class ISimulator {
public:
    virtual ~ISimulator() = default;
    virtual void timeStep(vector<Star>& stars) = 0;
    virtual void draw(RenderWindow& window, float scale) const {}
    virtual void toggleVisualization() {}
    virtual string getName() const = 0;
    virtual string getComplexity() const = 0;
};
#endif 
