.PHONY: up down build clean help check-libs native-run

# Цвета для вывода
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m # No Color

# Список необходимых библиотек
REQUIRED_LIBS := libpqxx libpq libX11 libXrandr libXcursor libXi libudev libGL libfreetype libopenal libFLAC libvorbis libsfml-graphics

help: ## Показать эту справку
	@echo "$(GREEN)N-Body Simulation - Доступные команды:$(NC)"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  $(YELLOW)%-15s$(NC) %s\n", $$1, $$2}'
	@echo ""

check-libs: ## Проверить наличие библиотек
	@echo "$(YELLOW)Проверка библиотек...$(NC)"
	@missing=0; \
	for lib in $(REQUIRED_LIBS); do \
		if ! ldconfig -p | grep -q "$$lib"; then \
			echo "$(RED)✗ $$lib не найдена$(NC)"; \
			missing=$$((missing + 1)); \
		fi; \
	done; \
	if [ $$missing -eq 0 ]; then \
		echo "$(GREEN)✓ Все библиотеки установлены$(NC)"; \
		exit 0; \
	else \
		echo "$(RED)✗ Не хватает $$missing библиотек$(NC)"; \
		exit 1; \
	fi

install-deps: ## Установить зависимости для нативного запуска
	@echo "$(YELLOW)Установка зависимостей...$(NC)"
	@if command -v apt-get > /dev/null 2>&1; then \
		echo "$(GREEN)Обнаружен APT (Ubuntu/Debian)$(NC)"; \
		sudo apt-get update && sudo apt-get install -y \
			build-essential cmake git \
			libpqxx-dev libpq-dev \
			libx11-dev libxrandr-dev libxcursor-dev libxi-dev \
			libudev-dev libgl1-mesa-dev libfreetype6-dev \
			libopenal-dev libflac-dev libvorbis-dev \
			libsfml-dev; \
	elif command -v dnf > /dev/null 2>&1; then \
		echo "$(GREEN)Обнаружен DNF (Fedora)$(NC)"; \
		sudo dnf install -y \
			gcc-c++ cmake git \
			libpqxx-devel postgresql-devel \
			libX11-devel libXrandr-devel libXcursor-devel libXi-devel \
			systemd-devel mesa-libGL-devel freetype-devel \
			openal-soft-devel flac-devel libvorbis-devel \
			SFML-devel; \
	elif command -v pacman > /dev/null 2>&1; then \
		echo "$(GREEN)Обнаружен Pacman (Arch)$(NC)"; \
		sudo pacman -S --needed \
			gcc cmake git \
			libpqxx postgresql-libs \
			libx11 libxrandr libxcursor libxi \
			systemd-libs mesa freetype2 \
			openal flac libvorbis \
			sfml; \
	else \
		echo "$(RED)Неизвестный пакетный менеджер. Установите библиотеки вручную.$(NC)"; \
		exit 1; \
	fi
	@echo "$(GREEN)✓ Зависимости установлены$(NC)"

db-only: ## Запустить только базу данных для нативного запуска
	@echo "$(YELLOW)Запуск PostgreSQL...$(NC)"
	@sudo docker compose up -d db
	@echo "$(GREEN)✓ База данных запущена на порту 5433$(NC)"

native-run: ## Запустить нативно (если библиотеки есть) или через Docker
	@if $(MAKE) check-libs > /dev/null 2>&1; then \
		echo "$(GREEN)✓ Запуск нативной версии...$(NC)"; \
		if ! sudo docker compose ps db | grep -q "Up"; then \
			echo "$(YELLOW)Запуск базы данных...$(NC)"; \
			$(MAKE) db-only; \
			sleep 3; \
		fi; \
		if [ ! -f "build/nbody_simulation" ]; then \
			echo "$(YELLOW)Сборка проекта...$(NC)"; \
			mkdir -p build && cd build && \
			cmake -DENABLE_AUTH=ON -DCMAKE_BUILD_TYPE=Release .. && \
			make -j$$(nproc); \
		fi; \
		set -a && [ -f .env ] && . ./.env && set +a && ./build/nbody_simulation; \
	else \
		echo "$(YELLOW)Библиотеки не найдены.$(NC)"; \
		echo "$(YELLOW)Запустите 'make install-deps' для установки или 'make up' для Docker$(NC)"; \
		exit 1; \
	fi

up: 
	@xhost +local:docker > /dev/null 2>&1 || true
	@sudo docker compose up --abort-on-container-exit || true
	@sudo docker compose down

down:@sudo docker compose down
	@xhost -local:docker > /dev/null 2>&1 || true
	
build: ## Пересобрать образы
	@sudo docker compose build

clean: ## Полная очистка (удалить volumes и образы)
	@echo "ВНИМАНИЕ: Это удалит все данные БД!$(NC)"
	@read -p "Продолжить? [y/N] " -n 1 -r; \
	echo; \
	if [[ $$REPLY =~ ^[Yy]$$ ]]; then \
		echo "$(YELLOW)🧹 Очистка...$(NC)"; \
		sudo docker compose down -v; \
		sudo docker system prune -f; \
		echo "$(GREEN)✓ Готово!$(NC)"; \
	else \
		echo "$(YELLOW)Отменено$(NC)"; \
	fi

logs: ## Показать логи
	@sudo docker compose logs -f

restart: down up ## Перезапустить симуляцию
