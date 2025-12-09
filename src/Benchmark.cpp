#include "Benchmark.h"
#include "Utils.h"
#include "RuStrings.h"
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

    // Create galaxy once and reuse for fair comparison
    auto stars = createGalaxy(num_particles, 500.0, 800.0, 800.0);
    
    // Copy stars for second simulation to ensure identical initial state
    auto stars2 = stars;

    auto res1 = run(sim1, stars, num_steps);
    auto res2 = run(sim2, stars2, num_steps);

    bool isBrute1 = (sim1->getName().find("Brute") != string::npos);
    bool isBrute2 = (sim2->getName().find("Brute") != string::npos);
    string algo1Ru = isBrute1 ? Strings::BRUTE_FORCE : Strings::BARNES_HUT;
    string algo2Ru = isBrute2 ? Strings::BRUTE_FORCE : Strings::BARNES_HUT;

    ss << Strings::BENCHMARK_RESULTS << "\n\n";

    ss << "1. " << algo1Ru << "\n";
    ss << "   " << Strings::TIME << res1.total_duration_ms << " ms\n";
    ss << "   " << Strings::FPS_LABEL << fixed << setprecision(1) << res1.fps_equivalent << "\n\n";

    ss << "2. " << algo2Ru << "\n";
    ss << "   " << Strings::TIME << res2.total_duration_ms << " ms\n";
    ss << "   " << Strings::FPS_LABEL << fixed << setprecision(1) << res2.fps_equivalent << "\n\n";

    ss << Strings::VERDICT << ":\n";
    if (res1.avg_time_per_step_ms < res2.avg_time_per_step_ms)
    {
        double speedup = res2.avg_time_per_step_ms / res1.avg_time_per_step_ms;
        ss << algo1Ru << " is " << fixed << setprecision(2)
           << speedup << "x " << Strings::FASTER;
    }
    else
    {
        double speedup = res1.avg_time_per_step_ms / res2.avg_time_per_step_ms;
        ss << algo2Ru << " is " << fixed << setprecision(2)
           << speedup << "x " << Strings::FASTER;
    }

    return ss.str();
}
