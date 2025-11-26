#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "ISimulator.h"
#include "Star.h"
#include <string>
#include <vector>
#include <chrono>

struct BenchmarkResult {
    std::string algorithm_name;
    std::string complexity;
    int num_particles;
    int num_steps;
    long long total_duration_ms;
    double avg_time_per_step_ms;
    double fps_equivalent;
    int num_threads;
    
    void print() const;
    void saveToFile(const std::string& filename) const;
};

class Benchmark {
public:
    // Запустить бенчмарк для одного симулятора
    static BenchmarkResult run(ISimulator* simulator, 
                              std::vector<Star>& stars,
                              int num_steps);
    
    // Сравнить два симулятора
    static void compare(ISimulator* sim1, 
                       ISimulator* sim2,
                       int num_particles,
                       int num_steps,
                       const std::string& output_file = "benchmark_comparison.txt");

    // Новая функция для получения результатов сравнения в виде строки
    static std::string getComparisonResult(ISimulator* sim1, ISimulator* sim2, 
                                         int num_particles, int num_steps);
};

#endif // BENCHMARK_H
