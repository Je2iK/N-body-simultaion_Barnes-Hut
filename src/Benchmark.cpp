#include "Benchmark.h"
#include "Utils.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;

BenchmarkResult Benchmark::run(ISimulator *simulator, vector<Star> &stars, int num_steps)
{
    auto start_time = chrono::high_resolution_clock::now();

    for (int i = 0; i < num_steps; ++i)
    {
        simulator->timeStep(stars);
    }

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    BenchmarkResult result;
    result.algorithm_name = simulator->getName();
    result.complexity = simulator->getComplexity();
    result.num_particles = stars.size();
    result.num_threads = Simulation::NUM_THREADS;
    result.num_steps = num_steps;
    result.total_duration_ms = duration.count();
    result.avg_time_per_step_ms = static_cast<double>(result.total_duration_ms) / num_steps;
    result.fps_equivalent = 1000.0 / result.avg_time_per_step_ms;

    return result;
}

string Benchmark::getComparisonResult(ISimulator *sim1, ISimulator *sim2,
                                           int num_particles, int num_steps)
{
    stringstream ss;

    auto stars = createGalaxy(num_particles, 500.0, 800.0, 800.0);
    
    auto stars2 = stars;

    auto res1 = run(sim1, stars, num_steps);
    auto res2 = run(sim2, stars2, num_steps);

    bool isBrute1 = (sim1->getName().find("Brute") != string::npos);
    bool isBrute2 = (sim2->getName().find("Brute") != string::npos);
    string algo1Ru = isBrute1 ? u8"Полный перебор" : u8"Барнс-Хат";
    string algo2Ru = isBrute2 ? u8"Полный перебор" : u8"Барнс-Хат";

    ss << u8"РЕЗУЛЬТАТЫ ТЕСТА" << "\n\n";

    ss << "1. " << algo1Ru << "\n";
    ss << "   " << u8"Время: " << res1.total_duration_ms << " ms\n";
    ss << "   " << "FPS: " << fixed << setprecision(1) << res1.fps_equivalent << "\n\n";

    ss << "2. " << algo2Ru << "\n";
    ss << "   " << u8"Время: " << res2.total_duration_ms << " ms\n";
    ss << "   " << "FPS: " << fixed << setprecision(1) << res2.fps_equivalent << "\n\n";

    ss << u8"ВЕРДИКТ" << ":\n";
    if (res1.avg_time_per_step_ms < res2.avg_time_per_step_ms)
    {
        double speedup = res2.avg_time_per_step_ms / res1.avg_time_per_step_ms;
        ss << algo1Ru << " is " << fixed << setprecision(2)
           << speedup << "x " << u8"БЫСТРЕЕ";
    }
    else
    {
        double speedup = res1.avg_time_per_step_ms / res2.avg_time_per_step_ms;
        ss << algo2Ru << " is " << fixed << setprecision(2)
           << speedup << "x " << u8"БЫСТРЕЕ";
    }

    return ss.str();
}

void Benchmark::saveResult(const BenchmarkResult& result, const string& filename) {
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        file << "----------------------------------------\n";
        file << "Algorithm: " << result.algorithm_name << "\n";
        file << "Particles: " << result.num_particles << "\n";
        file << "Steps: " << result.num_steps << "\n";
        file << "Duration: " << result.total_duration_ms << " ms\n";
        file << "Avg time/step: " << result.avg_time_per_step_ms << " ms\n";
        file << "FPS Equivalent: " << result.fps_equivalent << "\n";
        file << "Threads: " << result.num_threads << "\n";
        file << "Complexity: " << result.complexity << "\n";
        
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        file << "Date: " << put_time(localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n";
        file << "----------------------------------------\n\n";
        
        cout << "Benchmark result saved to " << filename << endl;
    } else {
        cerr << "Failed to open file for saving benchmark result: " << filename << endl;
    }
}
