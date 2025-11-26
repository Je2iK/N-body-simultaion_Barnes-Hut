# Архитектура модульной N-Body симуляции

## Диаграмма классов

```
┌─────────────────────────────────────────────────────────────────┐
│                         ISimulator                              │
│                        (Interface)                              │
├─────────────────────────────────────────────────────────────────┤
│ + timeStep(stars: vector<Star>&): void                         │
│ + draw(window: RenderWindow&, scale: float): void              │
│ + toggleVisualization(): void                                   │
│ + getName(): string                                             │
│ + getComplexity(): string                                       │
└─────────────────────────────────────────────────────────────────┘
                              △
                              │
                 ┌────────────┴────────────┐
                 │                         │
    ┌────────────┴─────────────┐  ┌───────┴──────────────┐
    │  BarnesHutSimulator      │  │ BruteForceSimulator  │
    ├──────────────────────────┤  ├──────────────────────┤
    │ - root: Cell*            │  │ - area_width: double │
    │ - area_width: double     │  │ - area_height: double│
    │ - area_height: double    │  ├──────────────────────┤
    │ - show_tree: bool        │  │ + timeStep()         │
    │ - tree_built: atomic     │  │ + getName()          │
    ├──────────────────────────┤  │ + getComplexity()    │
    │ + buildTree()            │  └──────────────────────┘
    │ + insert()               │
    │ + subdivide()            │
    │ + calculateAccel...()    │
    │ + timeStep()             │
    │ + draw()                 │
    │ + toggleVisualization()  │
    │ + getName()              │
    │ + getComplexity()        │
    └──────────────────────────┘
                │
                │ uses
                ▼
         ┌─────────────┐
         │    Cell     │
         ├─────────────┤
         │ + x, y      │
         │ + width     │
         │ + height    │
         │ + mass      │
         │ + com_x     │
         │ + com_y     │
         │ + stars     │
         │ + children  │
         │ + is_divided│
         ├─────────────┤
         │ + draw()    │
         └─────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                         Benchmark                               │
├─────────────────────────────────────────────────────────────────┤
│ + run(sim: ISimulator*, stars, steps): BenchmarkResult         │
│ + compare(sim1, sim2, particles, steps, file): void            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ uses
                              ▼
                    ┌──────────────────┐
                    │ BenchmarkResult  │
                    ├──────────────────┤
                    │ + algorithm_name │
                    │ + complexity     │
                    │ + num_particles  │
                    │ + num_steps      │
                    │ + total_duration │
                    │ + avg_time       │
                    │ + fps_equivalent │
                    ├──────────────────┤
                    │ + print()        │
                    │ + saveToFile()   │
                    └──────────────────┘

┌─────────────────┐
│      Star       │
├─────────────────┤
│ + x, y          │
│ + vx, vy        │
│ + mass          │
│ + color         │
│ + radius        │
└─────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                         Utils                                   │
├─────────────────────────────────────────────────────────────────┤
│ + fast_inv_sqrt(x: double): double                             │
│ + createGalaxy(radius, cx, cy, accel_factor): vector<Star>     │
│ + drawStars(window, stars, scale): void                         │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                       Constants                                 │
├─────────────────────────────────────────────────────────────────┤
│ Physics::                                                       │
│   - G, EPSILON_SQ, BLACK_HOLE_MASS, etc.                       │
│ BarnesHut::                                                     │
│   - DT, THETA, MIN_CELL_SIZE                                   │
│ BruteForce::                                                    │
│   - DT                                                          │
│ Simulation::                                                    │
│   - NUM_STARS, NUM_THREADS, NUM_STEPS_BENCHMARK                │
│ Visual::                                                        │
│   - BACKGROUND_COLOR, TREE_COLOR, STAR_COLORS                  │
└─────────────────────────────────────────────────────────────────┘
```

## Поток данных

```
┌──────────┐
│   main   │
└────┬─────┘
     │
     ├─── Создает ──→ BarnesHutSimulator или BruteForceSimulator
     │
     ├─── Создает ──→ Galaxy (через createGalaxy())
     │
     └─── Запускает ──→ Benchmark::compare()
                            │
                            ├─→ run(sim1, stars1, steps)
                            │      │
                            │      ├─→ sim1->timeStep() × N
                            │      │      │
                            │      │      ├─→ buildTree() [Barnes-Hut]
                            │      │      │      │
                            │      │      │      └─→ insert() → subdivide()
                            │      │      │
                            │      │      └─→ calculateAccelerations()
                            │      │             │
                            │      │             └─→ Parallel threads
                            │      │
                            │      └─→ return BenchmarkResult
                            │
                            ├─→ run(sim2, stars2, steps)
                            │
                            └─→ Сравнение и вывод результатов
```

## Модульная структура файлов

```
include/
├── Star.h              ← Базовая структура данных
├── Constants.h         ← Все константы (namespace-based)
├── Utils.h             ← Вспомогательные функции
├── Cell.h              ← Quadtree ячейка (для Barnes-Hut)
├── ISimulator.h        ← Интерфейс симулятора
├── BarnesHutSimulator.h ← Реализация O(N log N)
├── BruteForceSimulator.h ← Реализация O(N²)
└── Benchmark.h         ← Система бенчмаркинга

src/
├── main.cpp            ← Точка входа + меню
├── Utils.cpp           ← Реализация утилит
├── Cell.cpp            ← Реализация Cell
├── BarnesHutSimulator.cpp ← Реализация Barnes-Hut
├── BruteForceSimulator.cpp ← Реализация Brute Force
└── Benchmark.cpp       ← Реализация бенчмарков
```

## Принципы дизайна

### 1. **Разделение интерфейса и реализации**
- `ISimulator` определяет контракт
- Конкретные симуляторы реализуют детали

### 2. **Single Responsibility Principle**
- `Star` - только данные о частице
- `Cell` - только структура quadtree
- `Benchmark` - только измерение производительности
- `Utils` - только вспомогательные функции

### 3. **Open/Closed Principle**
- Легко добавить новый алгоритм (наследуя ISimulator)
- Не нужно менять существующий код

### 4. **Dependency Inversion**
- `Benchmark` зависит от `ISimulator`, а не от конкретных реализаций
- `main` работает с абстракцией, а не с деталями

### 5. **Namespace Organization**
- `Physics::` - физические константы
- `BarnesHut::` - специфичные для Barnes-Hut
- `BruteForce::` - специфичные для Brute Force
- `Simulation::` - общие параметры симуляции
- `Visual::` - визуализация

## Расширяемость

### Добавление нового алгоритма (например, FMM - Fast Multipole Method):

1. Создать `include/FMMSimulator.h`:
```cpp
class FMMSimulator : public ISimulator {
    // Реализация FMM
};
```

2. Создать `src/FMMSimulator.cpp`

3. Добавить в `main.cpp`:
```cpp
case 6: {
    auto simulator = std::make_unique<FMMSimulator>(AREA_SIZE, AREA_SIZE);
    runVisualization(simulator.get(), "N-Body - FMM");
    break;
}
```

4. Обновить `Makefile_new` и `CMakeLists_new.txt`

Готово! Новый алгоритм интегрирован.

## Многопоточность

```
Main Thread
    │
    ├─→ Thread Pool (NUM_THREADS)
    │      │
    │      ├─→ Thread 1: particles [1...N/T]
    │      ├─→ Thread 2: particles [N/T...2N/T]
    │      ├─→ Thread 3: particles [2N/T...3N/T]
    │      └─→ Thread T: particles [(T-1)N/T...N]
    │
    └─→ Join all threads
```

Каждый поток вычисляет ускорения для своего подмножества частиц.
