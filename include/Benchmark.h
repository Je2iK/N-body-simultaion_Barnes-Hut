#ifndef BENCHMARK_H
#define BENCHMARK_H
#include "ISimulator.h"
#include "Star.h"
#include <string>
#include <vector>
#include <chrono>
using namespace std;
struct BenchmarkResult {
    string algorithm_name;
    string complexity;
    int num_particles;
    int num_steps;
    long long total_duration_ms;
    double avg_time_per_step_ms;
    double fps_equivalent;
    int num_threads;
};
class Benchmark {
public:
    static BenchmarkResult run(ISimulator* simulator, 
                              vector<Star>& stars,
                              int num_steps);
    static string getComparisonResult(ISimulator* sim1, ISimulator* sim2, 
                                         int num_particles, int num_steps);
    static void saveResult(const BenchmarkResult& result, const string& filename = "benchmark_results.txt");
};
#endif 
