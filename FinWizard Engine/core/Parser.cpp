#include "Parser.h"

std::optional<Task> ConfigParser::parse(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt; // кидаем пустой обьект, тк std::optional
    }

    json config;

    try {
        file >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "Ошибка парсинга JSON в" << path << ": " << e.what() << "\n";
        return std::nullopt;
    }
    // Проверка Обьекта
    if (!config.is_object()) {
        std::cerr << "Конфиг должен быть обьектом!\n";
        return std::nullopt;
    }
    // Проверка workflow_id
    if (!config.contains("workflow_id") || !config["workflow_id"].is_string()) {
        std::cerr << "Нет workflow_id или он не строка\n";
        return std::nullopt;
    }
    // Проверка version
    if (!config.contains("version") || !config["version"].is_string()) {
        std::cerr << "Нет version\n";
        return std::nullopt;
    }
    // Проверка steps
    if (!config.contains("steps") || !config["steps"].is_array()) {
        std::cerr << "Нет steps или steps не массив\n";
        return std::nullopt;
    }

    // Заполянем Task
    Task task;

    // корневые поля
    task.workflow_id = config["workflow_id"].get<std::string>();
    task.version = config.value("version", "1.0");

    // Настройки если есть
    if (config.contains("settings") && config["settings"].is_object()) {
        const auto& s = config["settings"];
        task.settings.stop_on_failure = s.value("stop_on_failure", true);
        task.settings.default_timeout_seconds = s.value("default_timeout_seconds", 300);
        task.settings.on_failure_action = s.value("on_failure_action", "");
    }

    // Парсим шаги
    const auto& steps_json = config["steps"];

    for (const auto& s : steps_json) {
        Step step;

        // обязательные поля
        if (!s.contains("executor") || !s["executor"].is_string()) {
            std::cerr << "Шаг без executor или executor не строка\n";
            return std::nullopt;
        }
        step.executor = s["executor"].get<std::string>();

        // Опциональные поля
        step.id          = s.value("id", "");
        step.description = s.value("description", "");
        step.parameters  = s.value("parameters", json::object());
        step.save_as     = s.value("save_as", "");
        step.depends_on  = s.value("depends_on", std::vector<std::string>{});
        step.timeout_seconds = s.value("timeout_seconds", task.settings.default_timeout_seconds);

        task.steps.push_back(step);
    }
    if (task.steps.empty()) {
        std::cerr << "Нет валидных шагов в конфиге\n";
        return std::nullopt;
    }
    return task;
}
