#ifndef BRUTE_FORCE_SIMULATOR_H
#define BRUTE_FORCE_SIMULATOR_H

#include "ISimulator.h"

class BruteForceSimulator : public ISimulator {
private:
    double area_width, area_height;
    
    std::pair<double, double> calculateN2Acceleration(
        const std::vector<Star>& stars, size_t star_index) const;
    
    void calculateAccelerationsParallel(
        const std::vector<Star>& stars,
        std::vector<double>& acc_x,
        std::vector<double>& acc_y) const;

public:
    BruteForceSimulator(double width, double height);
    
    void timeStep(std::vector<Star>& stars) override;
    
    std::string getName() const override { return "Brute Force (N²)"; }
    std::string getComplexity() const override { return "O(N²)"; }
};

#endif // BRUTE_FORCE_SIMULATOR_H
