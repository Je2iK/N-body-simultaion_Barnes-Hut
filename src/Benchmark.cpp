#include "Benchmark.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

BenchmarkResult Benchmark::run(ISimulator* simulator, std::vector<Star>& stars, int num_steps) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_steps; ++i) {
        simulator->timeStep(stars);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
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

void BenchmarkResult::print() const {
    std::cout << "\n========================================\n";
    std::cout << "  Benchmark Results: " << algorithm_name << "\n";
    std::cout << "========================================\n";
    std::cout << "Algorithm:           " << algorithm_name << "\n";
    std::cout << "Complexity:          " << complexity << "\n";
    std::cout << "Particles:           " << num_particles << "\n";
    std::cout << "Threads:             " << num_threads << "\n";
    std::cout << "Steps:               " << num_steps << "\n";
    std::cout << "Total duration:      " << total_duration_ms << " ms\n";
    std::cout << "Avg time per step:   " << avg_time_per_step_ms << " ms\n";
    std::cout << "Equivalent FPS:      " << fps_equivalent << "\n";
    std::cout << "========================================\n\n";
}

void BenchmarkResult::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "Algorithm: " << algorithm_name << "\n";
        file << "Particles: " << num_particles << "\n";
        file << "Steps: " << num_steps << "\n";
        file << "Total Time: " << total_duration_ms << " ms\n";
        file << "FPS: " << fps_equivalent << "\n";
        file << "----------------------------------------\n";
        file.close();
        std::cout << "📊 Results saved to: " << filename << "\n";
    }
}

void Benchmark::compare(ISimulator* sim1, ISimulator* sim2, 
                       int num_particles, int num_steps,
                       const std::string& output_file) {
    std::cout << "\n╔════════════════════════════════════════════════════╗\n";
    std::cout << "║     N-Body Simulation Benchmark Comparison         ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
    
    // Sim 1
    std::cout << "Running benchmark for " << sim1->getName() << " (" << num_steps << " steps)...\n";
    auto stars1 = createGalaxy(500.0, 800.0, 800.0); // Temp galaxy
    auto res1 = run(sim1, stars1, num_steps);
    res1.print();
    
    // Sim 2
    std::cout << "Running benchmark for " << sim2->getName() << " (" << num_steps << " steps)...\n";
    auto stars2 = createGalaxy(500.0, 800.0, 800.0); // Temp galaxy
    auto res2 = run(sim2, stars2, num_steps);
    res2.print();
    
    // Comparison
    std::cout << "\n╔════════════════════════════════════════════════════╗\n";
    std::cout << "║                   COMPARISON                       ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n\n";
    
    std::cout << sim1->getName() << " vs " << sim2->getName() << ":\n";
    std::cout << "  " << sim1->getName() << ": " << res1.avg_time_per_step_ms << " ms/step\n";
    std::cout << "  " << sim2->getName() << ": " << res2.avg_time_per_step_ms << " ms/step\n\n";
    
    if (res1.avg_time_per_step_ms < res2.avg_time_per_step_ms) {
        double speedup = res2.avg_time_per_step_ms / res1.avg_time_per_step_ms;
        std::cout << "✅ " << sim1->getName() << " is " << std::fixed << std::setprecision(2) 
                  << speedup << "x FASTER than " << sim2->getName() << "\n";
    } else {
        double speedup = res1.avg_time_per_step_ms / res2.avg_time_per_step_ms;
        std::cout << "✅ " << sim2->getName() << " is " << std::fixed << std::setprecision(2) 
                  << speedup << "x FASTER than " << sim1->getName() << "\n";
    }
    
    std::ofstream file(output_file);
    if (file.is_open()) {
        file << "Comparison Result:\n";
        file << sim1->getName() << ": " << res1.fps_equivalent << " FPS\n";
        file << sim2->getName() << ": " << res2.fps_equivalent << " FPS\n";
        file.close();
        std::cout << "\n📊 Results saved to: " << output_file << "\n";
    }
}

std::string Benchmark::getComparisonResult(ISimulator* sim1, ISimulator* sim2, 
                                         int num_particles, int num_steps) {
    std::stringstream ss;
    
    // Sim 1
    auto stars1 = createGalaxy(500.0, 800.0, 800.0);
    auto res1 = run(sim1, stars1, num_steps);
    
    // Sim 2
    auto stars2 = createGalaxy(500.0, 800.0, 800.0);
    auto res2 = run(sim2, stars2, num_steps);
    
    ss << "BENCHMARK RESULTS\n\n";
    
    ss << "1. " << sim1->getName() << "\n";
    ss << "   Time: " << res1.total_duration_ms << " ms\n";
    ss << "   FPS:  " << std::fixed << std::setprecision(1) << res1.fps_equivalent << "\n\n";
    
    ss << "2. " << sim2->getName() << "\n";
    ss << "   Time: " << res2.total_duration_ms << " ms\n";
    ss << "   FPS:  " << std::fixed << std::setprecision(1) << res2.fps_equivalent << "\n\n";
    
    ss << "VERDICT:\n";
    if (res1.avg_time_per_step_ms < res2.avg_time_per_step_ms) {
        double speedup = res2.avg_time_per_step_ms / res1.avg_time_per_step_ms;
        ss << sim1->getName() << " is " << std::fixed << std::setprecision(2) 
           << speedup << "x FASTER";
    } else {
        double speedup = res1.avg_time_per_step_ms / res2.avg_time_per_step_ms;
        ss << sim2->getName() << " is " << std::fixed << std::setprecision(2) 
           << speedup << "x FASTER";
    }
    
    return ss.str();
}
