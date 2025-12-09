#ifndef BARNES_HUT_SIMULATOR_H
#define BARNES_HUT_SIMULATOR_H

#include "ISimulator.h"
#include "Cell.h"
#include "Constants.h"
#include <memory>
#include <atomic>

using namespace std;
using namespace sf;

class BarnesHutSimulator : public ISimulator {
private:
    unique_ptr<Cell> root;
    double area_width, area_height;
    bool show_tree;
    atomic<bool> tree_built;
    double theta;  // Barnes-Hut theta parameter
    double min_cell_size;  // Minimum cell size for subdivision
    
    void resetTree(double x, double y, double w, double h);
    void buildTree(const vector<Star>& stars);
    void insert(const Star* star);
    void subdivide(Cell* cell);
    int getQuadrant(const Star* star, const Cell* cell) const;
    
    void recalculateCellMassAndCOM(Cell* cell);
    void updateCellMassAndCOM(Cell* cell, const Star* star);
    
    pair<double, double> calculateTreeAccelerationRecursive(
        const Star* star, const Cell* cell) const;
    
    void calculateAccelerationsParallel(
        const vector<Star>& stars,
        vector<double>& acc_x,
        vector<double>& acc_y) const;

public:
    BarnesHutSimulator(double width, double height, double theta = BarnesHut::THETA);
    
    void timeStep(vector<Star>& stars) override;
    void draw(RenderWindow& window, float scale) const override;
    void toggleVisualization() override;
    
    // Theta control
    void increaseTheta() { theta = min(theta + 0.1, 2.0); }
    void decreaseTheta() { theta = max(theta - 0.1, 0.1); }
    double getTheta() const { return theta; }
    
    // Min cell size control
    void increaseMinCellSize() { min_cell_size = min(min_cell_size + 10.0, 100.0); }
    void decreaseMinCellSize() { min_cell_size = max(min_cell_size - 10.0, 1.0); }
    double getMinCellSize() const { return min_cell_size; }
    
    string getName() const override { return "Barnes-Hut"; }
    string getComplexity() const override { return "O(N log N)"; }
};

#endif // BARNES_HUT_SIMULATOR_H
