#include "BarnesHutSimulator.h"
#include "Constants.h"
#include "Utils.h"
#include <thread>
#include <algorithm>

using namespace Physics;
using namespace BarnesHut;
using namespace Simulation;

BarnesHutSimulator::BarnesHutSimulator(double width, double height)
    : area_width(width), area_height(height), show_tree(false), tree_built(false)
{
    resetTree();
}

void BarnesHutSimulator::resetTree() {
    root = std::make_unique<Cell>(area_width/2, area_height/2, area_width, area_height);
    tree_built = false;
}

void BarnesHutSimulator::buildTree(const std::vector<Star>& stars) {
    resetTree();
    for (const auto& star : stars) {
        insert(&star);
    }
    tree_built = true;
}

void BarnesHutSimulator::recalculateCellMassAndCOM(Cell* cell) {
    cell->mass = 0.0;
    cell->com_x = 0.0;
    cell->com_y = 0.0;
    
    if (cell->stars.empty()) return;
    
    double total_mass = 0.0;
    double sum_x = 0.0;
    double sum_y = 0.0;
    
    for (const auto* star : cell->stars) {
        sum_x += star->x * star->mass;
        sum_y += star->y * star->mass;
        total_mass += star->mass;
    }
    
    cell->mass = total_mass;
    if (total_mass > 0) {
        cell->com_x = sum_x / total_mass;
        cell->com_y = sum_y / total_mass;
    } else if (!cell->stars.empty()) {
        cell->com_x = cell->stars[0]->x;
        cell->com_y = cell->stars[0]->y;
    }
}

void BarnesHutSimulator::updateCellMassAndCOM(Cell* cell, const Star* star) {
    const double old_mass = cell->mass;
    const double new_mass = cell->mass + star->mass;
    
    if (new_mass > 0) {
        cell->com_x = (cell->com_x * old_mass + star->x * star->mass) / new_mass;
        cell->com_y = (cell->com_y * old_mass + star->y * star->mass) / new_mass;
    } else {
        cell->com_x = star->x;
        cell->com_y = star->y;
    }
    cell->mass = new_mass;
}

int BarnesHutSimulator::getQuadrant(const Star* star, const Cell* cell) const {
    if (star->x < cell->x) {
        return (star->y < cell->y) ? 0 : 2;
    } else {
        return (star->y < cell->y) ? 1 : 3;
    }
}

void BarnesHutSimulator::subdivide(Cell* cell) {
    if (cell->width < MIN_CELL_SIZE || cell->height < MIN_CELL_SIZE) {
        return;
    }
    
    const double half_width = cell->width / 2;
    const double half_height = cell->height / 2;
    const double quarter_width = half_width / 2;
    const double quarter_height = half_height / 2;
    
    cell->children.push_back(std::make_unique<Cell>(
        cell->x - quarter_width, cell->y - quarter_height, half_width, half_height));
    cell->children.push_back(std::make_unique<Cell>(
        cell->x + quarter_width, cell->y - quarter_height, half_width, half_height));
    cell->children.push_back(std::make_unique<Cell>(
        cell->x - quarter_width, cell->y + quarter_height, half_width, half_height));
    cell->children.push_back(std::make_unique<Cell>(
        cell->x + quarter_width, cell->y + quarter_height, half_width, half_height));
    
    cell->is_divided = true;
    
    auto old_stars = std::move(cell->stars);
    cell->stars.clear();
    
    for (auto* star : old_stars) {
        const int quadrant = getQuadrant(star, cell);
        if (quadrant >= 0 && quadrant < 4) {
            cell->children[quadrant]->stars.push_back(star);
        }
    }
    
    for (const auto& child : cell->children) {
        recalculateCellMassAndCOM(child.get());
    }
}

void BarnesHutSimulator::insert(const Star* star) {
    Cell* current = root.get();
    updateCellMassAndCOM(current, star);

    while (true) {
        if (!current->is_divided) {
            if (current->stars.empty()) {
                current->stars.push_back(const_cast<Star*>(star));
                return;
            } else if (current->stars.size() == 1) {
                subdivide(current);
                
                if (!current->is_divided) {
                    current->stars.push_back(const_cast<Star*>(star));
                    recalculateCellMassAndCOM(current);
                    return;
                }
            }
        }
        
        if (current->is_divided) {
            const int quadrant = getQuadrant(star, current);
            current = current->children[quadrant].get();
            updateCellMassAndCOM(current, star);
        } else {
            return;
        }
    }
}

std::pair<double, double> BarnesHutSimulator::calculateTreeAccelerationRecursive(
    const Star* star, const Cell* cell) const {
    if (cell == nullptr || cell->mass == 0.0) return {0, 0};
    
    const double dx_com = cell->com_x - star->x;
    const double dy_com = cell->com_y - star->y;
    const double distance_sq_com = dx_com*dx_com + dy_com*dy_com + EPSILON_SQ;
    const double distance_com = std::sqrt(distance_sq_com);

    if (!cell->is_divided || (cell->width / distance_com) < THETA) {
        if (cell->stars.size() == 1 && cell->stars[0] == star) {
            return {0, 0};
        }

        const double inv_distance_cubed = fast_inv_sqrt(distance_sq_com) * 
                                         fast_inv_sqrt(distance_sq_com) * 
                                         fast_inv_sqrt(distance_sq_com);
        const double acceleration_magnitude = G * cell->mass * inv_distance_cubed;
        
        return {acceleration_magnitude * dx_com, acceleration_magnitude * dy_com};
    } else {
        double acc_x = 0.0, acc_y = 0.0;
        for (const auto& child : cell->children) {
            auto [fx, fy] = calculateTreeAccelerationRecursive(star, child.get());
            acc_x += fx;
            acc_y += fy;
        }
        return {acc_x, acc_y};
    }
}

void BarnesHutSimulator::calculateAccelerationsParallel(
    const std::vector<Star>& stars,
    std::vector<double>& acc_x,
    std::vector<double>& acc_y) const {
    if (!tree_built) return;
    
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
                    auto [tree_ax, tree_ay] = calculateTreeAccelerationRecursive(&stars[j], root.get());
                    
                    acc_x[j] = tree_ax;
                    acc_y[j] = tree_ay;
                }
            });
        }
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void BarnesHutSimulator::timeStep(std::vector<Star>& stars) {
    std::vector<double> acc_x(stars.size(), 0.0);
    std::vector<double> acc_y(stars.size(), 0.0);
    
    // Шаг 1: Расчет ускорений a(t) в текущих позициях
    buildTree(stars);
    calculateAccelerationsParallel(stars, acc_x, acc_y);

    const size_t start_index = 0;  // Обновляем ВСЕ частицы, включая черную дыру
    const size_t num_particles = stars.size();

    if (num_particles == 0) return;
    
    const size_t chunk_size = (num_particles + NUM_THREADS - 1) / NUM_THREADS;
    std::vector<std::thread> update_threads;
    
    // Шаг 2: Обновление позиций x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt^2
    // и промежуточных скоростей v_temp = v(t) + 0.5*a(t)*dt
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
                    
                    // Сохраняем v(t) + 0.5*a(t)*dt для следующего шага
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
    buildTree(stars);
    calculateAccelerationsParallel(stars, acc_x, acc_y);

    // Шаг 4: Завершение обновления скоростей
    // v(t+dt) = v(t) + 0.5*(a(t) + a(t+dt))*dt
    // = [v(t) + 0.5*a(t)*dt] + 0.5*a(t+dt)*dt
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

void BarnesHutSimulator::draw(sf::RenderWindow& window, float scale) const {
    if (show_tree && tree_built) {
        root->draw(window, scale);
    }
}

void BarnesHutSimulator::toggleVisualization() {
    show_tree = !show_tree;
}
