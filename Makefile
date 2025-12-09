.PHONY: up down build run clean help

# Цвета для вывода
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m # No Color

help: ## Показать эту справку
	@echo "$(GREEN)N-Body Simulation - Доступные команды:$(NC)"
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "  $(YELLOW)%-15s$(NC) %s\n", $$1, $$2}'
	@echo ""

up: 
	@xhost +local:docker > /dev/null 2>&1 || true
	@sudo docker compose up --abort-on-container-exit

down:@sudo docker compose down
	@xhost -local:docker > /dev/null 2>&1 || true
	
build: ## Пересобрать образы
	@sudo docker compose build

run: up ## Алиас для up

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
