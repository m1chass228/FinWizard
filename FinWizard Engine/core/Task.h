#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Настройки выполнения задачи (глобальные для всего workflow)
struct WorkflowSettings {
    bool stop_on_failure = true;        // остановить всю задачу при ошибке шага
    int default_timeout_seconds = 300;  // общий таймаут на задачу (если не указан в шаге)
    std::string on_failure_action = ""; // "notify", "retry", "skip" и т.д.
    // Можно добавить: log_level, retry_policy и т.д.
};

// Один шаг задачи
struct Step {
    bool enabled = true;      // чтобы можно было
    std::string id;           // уникальный ID шага (для depends_on, логов, отладки)
    std::string executor;     // имя исполнителя ("fs.copy", "http_request", "python_script")
    std::string description;  // человекочитаемое описание (для логов/UI)
    json parameters;          // любые параметры шага (nlohmann::json — гибко)
    std::optional<std::string> save_as;      // ключ в context, куда сохранить результат шага (опционально)
    std::vector<std::string> depends_on;   // от каких шагов зависит (по id) — для будущего DAG
    std::optional<int> timeout_seconds = 0;               // таймаут конкретного шага (0 = без лимита)
    // Можно добавить: retry_count, retry_delay, on_failure, run_parallel и т.д.

};

// Вся задача (workflow)
struct Task {
    std::string workflow_id;   // уникальный ID задачи ("daily_report_gen")
    std::string version;       // версия конфига ("1.0")
    WorkflowSettings settings; // глобальные настройки
    std::vector<Step> steps;   // последовательность шагов
    // Можно добавить: std::string name, description, created_at и т.д.
};

st
