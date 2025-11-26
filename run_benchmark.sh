#!/bin/bash

echo "╔════════════════════════════════════════════════════╗"
echo "║   N-Body Simulation - Quick Benchmark Test         ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Проверка существования исполняемого файла
if [ ! -f "bin/nbody_simulation" ]; then
    echo "❌ Executable not found. Building..."
    make
    if [ $? -ne 0 ]; then
        echo "❌ Build failed!"
        exit 1
    fi
fi

echo "Running comparison benchmark..."
echo "This will compare Barnes-Hut vs Brute Force algorithms"
echo ""

# Запуск программы с автоматическим выбором опции 5 (сравнение)
echo "5" | ./bin/nbody_simulation

echo ""
echo "✅ Benchmark complete! Check benchmark_comparison.txt for results."
