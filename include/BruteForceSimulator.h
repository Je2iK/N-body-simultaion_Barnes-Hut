#ifndef BRUTE_FORCE_SIMULATOR_H
#define BRUTE_FORCE_SIMULATOR_H

#include "ISimulator.h"

using namespace std;

class BruteForceSimulator : public ISimulator {
private:
    double area_width, area_height;
    
    pair<double, double> calculateN2Acceleration(
        const vector<Star>& stars, size_t star_index) const;
    
    void calculateAccelerationsParallel(
        const vector<Star>& stars,
        vector<double>& acc_x,
        vector<double>& acc_y) const;

public:
    BruteForceSimulator(double width, double height);
    
    void timeStep(vector<Star>& stars) override;
    
    string getName() const override { return "Brute Force (N^2)"; }
    string getComplexity() const override { return "O(N^2)"; }
};

#endif // BRUTE_FORCE_SIMULATOR_H
