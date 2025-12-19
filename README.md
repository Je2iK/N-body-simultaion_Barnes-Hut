# N-Body Simulation (Barnes-Hut Algorithm)

Высокопроизводительная симуляция N-тел с использованием алгоритма Барнса-Хата для оптимизации вычислений гравитационных взаимодействий.

## Структура проекта

```
N-body-simultaion_Barnes-Hut/
├── src/                          # Исходный код
│   ├── main.cpp                  # Точка входа
│   ├── BarnesHutSimulator.cpp    # Реализация алгоритма Барнса-Хата
│   ├── BruteForceSimulator.cpp   # Реализация метода грубой силы
│   ├── Menu.cpp                  # Пользовательский интерфейс
│   ├── AuthManager.cpp           # Система аутентификации
│   ├── Benchmark.cpp             # Система бенчмарков
│   ├── Utils.cpp                 # Вспомогательные функции
│   └── Cell.cpp                  # Структура данных для квадродерева
├── include/                      # Заголовочные файлы
│   ├── BarnesHutSimulator.h
│   ├── BruteForceSimulator.h
│   ├── ISimulator.h
│   ├── Menu.h
│   ├── AuthManager.h
│   ├── Benchmark.h
│   ├── Utils.h
│   ├── Cell.h
│   ├── Star.h
│   └── Constants.h
├── database/                     # SQL схемы
│   └── init.sql
├── bin/                          # Скомпилированные исполняемые файлы
├── CMakeLists.txt               # Конфигурация CMake
├── Makefile                     # Автоматизация сборки
├── docker-compose.yml           # Docker конфигурация
├── Dockerfile                   # Docker образ
└── .env                         # Переменные окружения
```

## Требования

### Системные зависимости
- C++17 совместимый компилятор (GCC 7+, Clang 5+)
- CMake 3.14+
- PostgreSQL клиентские библиотеки
- SFML 2.5+
- OpenGL
- X11 (Linux)

### Для Docker
- Docker Engine 20.10+
- Docker Compose 2.0+


```bash
# Клонирование репозитория
git clone 
cd N-body-simultaion_Barnes-Hut

# Запуск симуляции
make up
```


## Команды Makefile

- `make up` - Запуск через Docker
- `make down` - Остановка контейнеров

## Алгоритмы

- **Barnes-Hut**: O(N log N) сложность, использует квадродерево для аппроксимации дальних взаимодействий
- **Brute Force**: O(N²) сложность, точные вычисления всех парных взаимодействий

## Производительность

Результаты бенчмарков сохраняются в `benchmark_results.txt`. Алгоритм Барнса-Хата показывает значительное преимущество при N > 1000 тел.
