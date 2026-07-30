SHELL := /bin/bash
COMPOSE := docker compose
OBS := -f docker-compose.yml -f docker-compose.observability.yml

.DEFAULT_GOAL := help

.PHONY: help
help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-18s\033[0m %s\n", $$1, $$2}'

.PHONY: up
up: ## Build and start the full stack
	$(COMPOSE) up --build

.PHONY: up-obs
up-obs: ## Start the stack with Prometheus + Grafana
	$(COMPOSE) $(OBS) up --build

.PHONY: down
down: ## Stop and remove containers
	$(COMPOSE) down -v

.PHONY: logs
logs: ## Tail logs
	$(COMPOSE) logs -f

.PHONY: smoke
smoke: ## Run the end-to-end smoke test against a running stack
	./scripts/smoke-test.sh

.PHONY: gateway
gateway: ## Build the C++ gateway locally
	cmake -S gateway -B gateway/build -DCMAKE_BUILD_TYPE=Release
	cmake --build gateway/build -j

.PHONY: test
test: test-gateway test-auth ## Run all unit tests

.PHONY: test-gateway
test-gateway: ## Build and run C++ gateway tests
	cmake -S gateway -B gateway/build -DAPIWAY_BUILD_TESTS=ON
	cmake --build gateway/build -j
	cd gateway/build && ctest --output-on-failure

.PHONY: test-auth
test-auth: ## Run Python auth-service tests
	cd services/auth-service && python -m pytest -q

.PHONY: dashboard
dashboard: ## Install deps and build the dashboard
	cd dashboard && npm install && npm run build

.PHONY: secret
secret: ## Print a fresh JWT secret
	./scripts/gen-secret.sh
