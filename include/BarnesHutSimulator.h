#ifndef BARNES_HUT_SIMULATOR_H
#define BARNES_HUT_SIMULATOR_H

#include "ISimulator.h"
#include "Cell.h"
#include <memory>
#include <atomic>

class BarnesHutSimulator : public ISimulator {
private:
    std::unique_ptr<Cell> root;
    double area_width, area_height;
    bool show_tree;
    std::atomic<bool> tree_built;
    
    void resetTree();
    void buildTree(const std::vector<Star>& stars);
    void insert(const Star* star);
    void subdivide(Cell* cell);
    int getQuadrant(const Star* star, const Cell* cell) const;
    
    void recalculateCellMassAndCOM(Cell* cell);
    void updateCellMassAndCOM(Cell* cell, const Star* star);
    
    std::pair<double, double> calculateTreeAccelerationRecursive(
        const Star* star, const Cell* cell) const;
    
    void calculateAccelerationsParallel(
        const std::vector<Star>& stars,
        std::vector<double>& acc_x,
        std::vector<double>& acc_y) const;

public:
    BarnesHutSimulator(double width, double height);
    
    void timeStep(std::vector<Star>& stars) override;
    void draw(sf::RenderWindow& window, float scale) const override;
    void toggleVisualization() override;
    
    std::string getName() const override { return "Barnes-Hut"; }
    std::string getComplexity() const override { return "O(N log N)"; }
};

#endif // BARNES_HUT_SIMULATOR_H
