#pragma once
#include <string>
#include <SFML/System/String.hpp>

using namespace std;
using namespace sf;

// Хелпер для создания String из UTF-8 строки
inline String ru(const string& s) {
    return String::fromUtf8(s.begin(), s.end());
}

// Все русские строки для приложения
namespace Strings {
    // Main Menu
    inline const string TITLE = "N-BODY";
    inline const string SUBTITLE = u8"СИМУЛЯЦИЯ";
    inline const string TAGLINE = u8"Алгоритм Барнса-Хата / Производительность O(N log N)";
    inline const string SIMULATIONS = u8"СИМУЛЯЦИИ";
    inline const string BENCHMARKS = u8"БЕНЧМАРКИ";
    
    // Buttons
    inline const string BARNES_HUT_SIM = u8"Симуляция\nБарнс-Хат";
    inline const string BRUTE_FORCE_SIM = u8"Симуляция\nПеребор";
    inline const string GALAXY_COLLISION = u8"Столкновение\nГалактик";
    inline const string BENCH_BARNES_HUT = u8"Тест алгоритма\nБарнс-Хата";
    inline const string BENCH_BRUTE_FORCE = u8"Тест алгоритма\nперебора";
    inline const string COMPARE_ALGOS = u8"Сравнить\nАлгоритмы";
    inline const string EXIT = u8"ВЫХОД";
    
    // Profile
    inline const string ACCOUNT = u8"АККАУНТ";
    inline const string LOGOUT = u8"Выйти";
    inline const string DELETE_ACCOUNT = u8"Удалить аккаунт";
    inline const string ADMIN_PANEL = u8"Админ панель";
    inline const string CHANGE_USERNAME = u8"Сменить имя";
    inline const string CHANGE_PASSWORD = u8"Сменить пароль";
    
    // Admin Panel
    inline const string USER_MANAGEMENT = u8"УПРАВЛЕНИЕ ПОЛЬЗОВАТЕЛЯМИ";
    inline const string SEARCH_PLACEHOLDER = u8"Поиск по имени...";
    inline const string BACK = u8"НАЗАД";
    inline const string SAVE = u8"СОХРАНИТЬ";
    inline const string CANCEL = u8"ОТМЕНА";
    inline const string ID = "ID";
    inline const string USERNAME = u8"ИМЯ";
    inline const string ROLE = u8"РОЛЬ";
    inline const string CREATED_AT = u8"СОЗДАН";
    inline const string ACTIONS = u8"ДЕЙСТВИЯ";
    inline const string PASSWORD_FOR = u8"Пароль для ";
    inline const string EDIT_USER_FOR = u8"Изменить имя для ";
    inline const string CHANGE_PASS_FOR = u8"Сменить пароль для ";
    inline const string DELETE_USER = u8"Удалить ";
    inline const string CANNOT_UNDO = u8"Это действие необратимо.";
    inline const string CLICK_TO_CLOSE = u8"Нажмите везде, чтобы закрыть";
    inline const string NOT_AVAILABLE_OLD = u8"Недоступно (старый)";
    inline const string NOT_AVAILABLE = u8"Недоступно";
    
    // Simulation
    inline const string PARTICLES = u8"Частицы";
    inline const string FPS = "FPS";
    inline const string THETA = u8"Тета";
    inline const string CELL = u8"Ячейка";
    inline const string PRESS_H = u8"H - Управление";
    inline const string PAUSED = u8"ПАУЗА";
    inline const string SELECTED = u8"ВЫБРАНО";
    inline const string MASS = u8"Масса: ";
    inline const string POSITION = u8"Позиция: ";
    inline const string VELOCITY = u8"Скорость: ";
    inline const string CONTROLS = u8"УПРАВЛЕНИЕ";
    inline const string PROCESSING = u8"Обработка...";
    inline const string RUNNING_BENCHMARK = u8"Запуск теста...\nПожалуйста, подождите.";
    inline const string PRESS_SPACE_ESC = u8"\n\nНажмите SPACE или ESC для возврата.";
    inline const string BENCHMARK = u8"БЕНЧМАРК";
    
    // Help
    inline const string HELP_SPACE = u8"SPACE       Пауза/Старт";
    inline const string HELP_E = u8"E           Следы вкл/выкл";
    inline const string HELP_R = u8"R           Центр на черной дыре";
    inline const string HELP_WASD = u8"WASD/Arrows Перемещение";
    inline const string HELP_ZOOM = u8"Колесо мыши Зум";
    inline const string HELP_CLICK = u8"ЛКМ         Выбрать звезду";
    inline const string HELP_T = u8"T           Сетка вкл/выкл";
    inline const string HELP_THETA = u8"+/-         Изменить Тета";
    inline const string HELP_CELL = u8"[/]         Размер ячейки";
    inline const string HELP_H = u8"H - Показать/Скрыть управление";
    inline const string HELP_ESC = u8"ESC - Вернуться в меню";
    
    // Benchmark GUI
    inline const string ALGORITHM = u8"Алгоритм: ";
    inline const string PARTICLES_LABEL = u8"Частицы: ";
    inline const string STEPS = u8"Шаги: ";
    inline const string TIME = u8"Время: ";
    inline const string FPS_LABEL = "FPS: ";
    inline const string BENCHMARK_RESULTS = u8"РЕЗУЛЬТАТЫ ТЕСТА";
    inline const string VERDICT = u8"ВЕРДИКТ";
    inline const string FASTER = u8"БЫСТРЕЕ";
    inline const string BARNES_HUT = u8"Барнс-Хат";
    inline const string BRUTE_FORCE = u8"Полный перебор";
    
    // Particle Selection
    inline const string SELECT_PARTICLE_COUNT = u8"Выберите кол-во частиц";
    inline const string START = u8"СТАРТ";
    inline const string PARTICLE_COUNT = u8"Количество частиц";
    
    // Auth & Profile
    inline const string NEW_USERNAME = u8"Новое имя";
    inline const string NEW_PASSWORD = u8"Новый пароль";
    inline const string USERNAME_LABEL = u8"Имя пользователя";
    inline const string PASSWORD_LABEL = u8"Пароль";
    inline const string LOG_IN = u8"ВОЙТИ";
    inline const string CREATE_ACCOUNT = u8"СОЗДАТЬ АККАУНТ";
    inline const string SIGN_UP = u8"РЕГИСТРАЦИЯ";
    inline const string BACK_TO_LOGIN = u8"НАЗАД";
    inline const string NBODY_SIMULATION = u8"N-BODY СИМУЛЯЦИЯ";
    inline const string BARNES_HUT_THETA = u8"Барнс-Хат Тета";
    inline const string CHOOSE_USERNAME = u8"Выберите имя";
    inline const string CHOOSE_PASSWORD = u8"Выберите пароль";
    inline const string CONFIRM = u8"ПОДТВЕРДИТЬ";
    inline const string DELETE_YOUR_ACCOUNT = u8"Удалить аккаунт?";
    
    // Errors
    inline const string INVALID_CREDENTIALS = u8"Неверное имя или пароль";
    inline const string TOO_SHORT = u8"Имя/Пароль слишком короткие";
    inline const string USERNAME_TAKEN = u8"Имя уже занято";
}
