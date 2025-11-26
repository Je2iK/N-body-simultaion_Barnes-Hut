#include "BruteForceSimulator.h"
#include "Constants.h"
#include "Utils.h"
#include <thread>
#include <algorithm>

using namespace Physics;
using namespace BruteForce;
using namespace Simulation;

BruteForceSimulator::BruteForceSimulator(double width, double height)
    : area_width(width), area_height(height)
{
}

std::pair<double, double> BruteForceSimulator::calculateN2Acceleration(
    const std::vector<Star>& stars, size_t star_index) const {
    double total_ax = 0.0;
    double total_ay = 0.0;
    const Star& current_star = stars[star_index];

    for (size_t j = 0; j < stars.size(); ++j) {
        if (star_index == j) continue;

        const Star& attracting_star = stars[j];
        
        const double dx = attracting_star.x - current_star.x;
        const double dy = attracting_star.y - current_star.y;
        
        const double distance_sq = dx*dx + dy*dy + EPSILON_SQ;
        
        const double inv_distance_cubed = fast_inv_sqrt(distance_sq) * 
                                         fast_inv_sqrt(distance_sq) * 
                                         fast_inv_sqrt(distance_sq);
        const double acceleration_magnitude = G * attracting_star.mass * inv_distance_cubed;
        
        total_ax += acceleration_magnitude * dx;
        total_ay += acceleration_magnitude * dy;
    }
    
    return {total_ax, total_ay};
}

void BruteForceSimulator::calculateAccelerationsParallel(
    const std::vector<Star>& stars,
    std::vector<double>& acc_x,
    std::vector<double>& acc_y) const {
    
    const size_t start_index = 0;
    const size_t num_particles = stars.size();

    if (num_particles == 0) return;
    
    const size_t chunk_size = (num_particles + NUM_THREADS - 1) / NUM_THREADS;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        const size_t start = start_index + i * chunk_size;
        const size_t end = std::min(start + chunk_size, stars.size());
        
        if (start < end) {
            threads.emplace_back([&, start, end]() {
                for (size_t j = start; j < end; ++j) {
                    auto [ax, ay] = calculateN2Acceleration(stars, j);
                    
                    acc_x[j] = ax;
                    acc_y[j] = ay;
                }
            });
        }
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void BruteForceSimulator::timeStep(std::vector<Star>& stars) {
    std::vector<double> acc_x(stars.size(), 0.0);
    std::vector<double> acc_y(stars.size(), 0.0);
    
    // Шаг 1: Расчет ускорений a(t) в текущих позициях
    calculateAccelerationsParallel(stars, acc_x, acc_y);

    const size_t start_index = 0;
    const size_t num_particles = stars.size();

    if (num_particles == 0) return;
    
    const size_t chunk_size = (num_particles + NUM_THREADS - 1) / NUM_THREADS;
    std::vector<std::thread> update_threads;
    
    // Шаг 2: Обновление позиций и промежуточных скоростей
    std::vector<double> vx_temp(stars.size());
    std::vector<double> vy_temp(stars.size());
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        const size_t start = start_index + i * chunk_size;
        const size_t end = std::min(start + chunk_size, stars.size());
        
        if (start < end) {
            update_threads.emplace_back([&, start, end]() {
                for (size_t j = start; j < end; ++j) {
                    const double ax = acc_x[j];
                    const double ay = acc_y[j];
                    
                    // Velocity Verlet: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
                    stars[j].x += stars[j].vx * DT + 0.5 * ax * DT * DT;
                    stars[j].y += stars[j].vy * DT + 0.5 * ay * DT * DT;
                    
                    // Сохраняем v(t) + 0.5*a(t)*dt
                    vx_temp[j] = stars[j].vx + 0.5 * ax * DT;
                    vy_temp[j] = stars[j].vy + 0.5 * ay * DT;
                }
            });
        }
    }
    
    for (auto& thread : update_threads) {
        thread.join();
    }
    update_threads.clear();

    // Шаг 3: Пересчет ускорений a(t+dt) в новых позициях
    calculateAccelerationsParallel(stars, acc_x, acc_y);

    // Шаг 4: Завершение обновления скоростей
    for (int i = 0; i < NUM_THREADS; ++i) {
        const size_t start = start_index + i * chunk_size;
        const size_t end = std::min(start + chunk_size, stars.size());
        
        if (start < end) {
            update_threads.emplace_back([&, start, end]() {
                for (size_t j = start; j < end; ++j) {
                    const double ax_new = acc_x[j];
                    const double ay_new = acc_y[j];
                    
                    // Velocity Verlet: v(t+dt) = v_temp + 0.5*a(t+dt)*dt
                    stars[j].vx = vx_temp[j] + 0.5 * ax_new * DT;
                    stars[j].vy = vy_temp[j] + 0.5 * ay_new * DT;
                }
            });
        }
    }
    
    for (auto& thread : update_threads) {
        thread.join();
    }
}
