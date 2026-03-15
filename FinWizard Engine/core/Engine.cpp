// core/Engine.cpp

#include <Engine.h>
// Для работы с файлами (чтение JSON-конфига)
#include <fstream>
#include <iostream>

// Это статическая переменная — общие "кнопки" (хелперы), которые мы отдаём всем плагинам
// Определяется один раз на всю программу
const ExecutorApi Engine::our_api = {
    // Указатель на функцию, которая читает строку из params
    .get_param_str = &Engine::static_get_param_str, 
    // Указатель на функцию, которая пишет строку в context
    .set_context_str = &Engine::static_set_context_str
}; 

// Функция, которую плагин будет вызывать, чтобы взять строку из параметров шага
// p_raw — это указатель на нашу структуру Params (мы его приводим к типу Params*)
// key — имя параметра ("from", "to" и т.д.)
// out — сюда плагин получит указатель на строку
int Engine::static_get_param_str(ParamsHandle* p_raw, const char* key, const char** out) {
    // Проверяем, что ничего не нулевое (защита от глупых ошибок)
    if (!p_raw || !key || !out) return 0;

    // Приводим "сырой" указатель к нашему типу Params*
    Params* p = static_cast<Params*>(p_raw);

    auto it = p->data.find(key);

    // если не нашли, возращаем 0
    if (it == p->data.end()) return 0;

    // Нашли - кладем адрес строки в out и возращаем 1(успех)
    *out = it->second.c_str();
    return 1;
}

// Функция, которую плагин вызывает, чтобы записать строку в общую память (context)
void Engine::static_set_context_str(ContextHandle* c_raw, const char* key, const char* value) {
    // Проверяем, что ничего не нулевое (защита от глупых ошибок)
    if (!c_raw || !key || !value) return;

    // Приводим сырой указатель к нашей структуре Context* 
    Context* c = static_cast<Context*>(c_raw);

    // Просто записываем значение по ключу в словарь
    c->data[key] = value;
    // Память под строку копируется автоматически (std::string делает копию)
}

// Конструктор — вызывается, когда мы пишем Engine engine;
Engine::Engine() {
    // Просто пишем в консоль, что движок создан (для отладки)
    std::cout << "[Engine] Создан\n";
}

// Деструктор — вызывается автоматически, когда engine умирает (конец функции или delete)
Engine::~Engine() {
    // Проходим по всем загруженным плагинам
    for (auto& [name, ex] : executors) {
        // Если есть таблица функций и объект — просим плагин удалить себя
        // значение (ex) — структура LoadedExecutor, в которой лежат три поля:
        // handle — указатель на загруженную библиотеку (.so / .dll)
        // instance — указатель на объект, который создал плагин (ExecutorHandle*)
        // vtable — указатель на таблицу функций плагина (execute, destroy и т.д.)
        if (ex.vtable && ex.instanse) {
            ex.vtable->destroy(ex.instanse);
        }
        // Если библиотека загружена — выгружаем её из памяти
        if (ex.handle) {
            DL_CLOSE(ex.handle);
        }
    }
    // Пишем в консоль, что всё почистили
    std::cout << "[Engine] Уничтожен\n";
}

// Метод загрузки одного (.so / .dll)
bool Engine::loadPlugin(const std::string& lib_path, const std::string& executor_name) {
    // Пытаемся открыть динамическую библиотеку
    Libhandle h = DL_OPEN(lib_path.c_str());
    if (!h) {
        // если не открылась - пишем ошибку и выходим
        std::cerr << "[Engine] Не удалось загрузить " << lib_path << ": " << DL_ERROR() << "\n";
        return false;
    }

    // Ищем в библиотеке функцию CreateExecutor
    CreateExecutorFn create = reinterpret_cast<CreateExecutorFn>(DL_SYM(h, "CreateExecutor"));
    // dl_sym возращает 
    // Указатель на функцию CreateExecutor внутри загруженной библиотеки (.so / .dll).
    // Тип возвращаемого значения — void* (сырой указатель, компилятор не знает, что это именно функция).
    // CreateExecutorFn — это typedef из executor_abi.h:
    // typedef ExecutorHandle* (*CreateExecutorFn)(
    //     const char*          executor_name,
    //     const ExecutorVTable** out_vtable,
    //     const ExecutorApi*   provided_api
    // );
    // Это указатель на функцию с конкретной сигнатурой (принимает 3 параметра, возвращает ExecutorHandle*).
    // reinterpret_cast<CreateExecutorFn>(...) говорит компилятору:
    // «Возьми этот void* (сырой указатель, который вернул dlsym/GetProcAddress),
    // и заставь компилятор думать, что это именно указатель на функцию типа CreateExecutorFn
    if (!create) {
        // Не нашли — ошибка, закрываем библиотеку
        std::cerr << "[Engine] CreateExecutor не найдена в " << lib_path << ": " << DL_ERROR() << "\n";
        DL_CLOSE(h);
        return false;
    }

    // Вызываем фабрику плагина — просим создать экземпляр executor'а
    const ExecutorVTable* vtable = nullptr;
    // Создаём пустой указатель vtable = nullptr — место, куда плагин положит свою таблицу функций.
    ExecutorHandle* instance = create(executor_name.c_str(), &vtable, &our_api);
    // Вызываем фабрику плагина create(...) и говорим ему:
    // «Будь executor'ом по имени fs.copy»
    // «Вот адрес переменной, куда положи свою таблицу функций (&vtable)»
    // «Вот мои хелперы для тебя (&our_api)»
    // ----- 
    // После этих двух строк у нас есть:
    // vtable — как звонить плагину (execute, destroy)
    // instance — кого именно звать (this-указатель для плагина)

    // Проверяем, что плагин вернул нормальный объект и таблицу функций
    if (!instance || !vtable) {
        std::cerr << "[Engine] Не удалось создать executor " << executor_name << "\n";
        DL_CLOSE(h);
        return false;
    }

    // Проверяем версию ABI — чтобы убедиться, что плагин совместим с нашим движком
    if (vtable->abi_version != EXECUTOR_ABI_VERSION) {
        std::cerr << "[Engine] Несовместимая версия ABI в " << executor_name << "\n";
        vtable->destroy(instance);
        DL_CLOSE(h);
        return false;
    }

    // Всё ок — сохраняем плагин в наш словарь
    executors[executor_name] = {h, instance, vtable};
    std::cout << "[Engine] Загружен executor: " << executor_name << " из " << lib_path << "\n";
    return true;
}

// Метод парсинга JSON-конфига
json Engine::parseConfig(const std::string& path) {
    // открываем файл на чтение
    std::ifstream file(path);
    if (!file.is_open()) {
        // не открылся - ошибка
        std::cerr << "[Engine] Не удалось открыть конфиг: " << path << "\n";
        return json(); // пустой json
    }

    json config;
    try {
        // пытаемся открыть и распарсить json
        file >> config;
    } catch (const json::parse_error& e) {
        // если json кривой - пишем ошибку
        std::cerr << "[Engine] Ошибка парсинга JSON: " << e.what() << "\n";
        return json();
    }

    // проверяем что в конфиге есть массив "steps"
    if (!config.contains("steps") || !config["steps"].is_array()) {
        std::cerr << "[Engine] Нет массива steps в конфиге\n";
        return json();
    }

    // Всё хорошо — возвращаем распарсенный конфиг
    return config;
}

// Выполнение одного шага из конфига
bool Engine::executeStep(const json& step, Context& ctx) {
    // Проверяем, что в шаге есть поле "executor" и оно строка
    if (!step.contains("executor") || !step["executor"].is_string()) {
        std::cerr << "[Engine] Шаг без executor\n";
        return false;
    }

    // Достаём имя executor'а ("fs.copy", "python.run" и т.д.)
    std::string name = step["executor"].get<std::string>();

    // Ищем его в загруженных плагинах
    auto it = executors.find(name);
    if (it == executors.end()) { // проверяем, не пустой ли it
        std::cerr << "[Engine] Executor не загружен: " << name << "\n";
        return false;
    }

    // Получаем данные плагина
    const auto& ex = it->second;
    if (!ex.vtable || !ex.instanse) {
        std::cerr << "[Engine] Повреждён executor: " << name << "\n";
        return false;
    }

    // Готовим параметры шага (Params)
    Params params;
    if (step.contains("params") && step["params"].is_object()) {
        // Проходим по всем ключам-значениям в "params"
    for (auto& [k, v] : step["params"].items()) {
            if (v.is_string()) {
                // Если значение — строка, кладём как есть
                params.data[k] = v.get<std::string>();
            } else {
                // Если не строка — превращаем в JSON-строку (временное решение)
                params.data[k] = v.dump();
            }
        }
    
    // Достаём ключ, под которым сохранить результат (если есть)
    std::string save_as = step.value("save_as", "");

    // Пишем в консоль, что начинаем шаг 
    std::cout << "[Engine] Выполняем шаг: " << name << "\n";

    // Самый важный вызов — звоним плагину
    ExecutorResult res = ex.vtable->execute(
        ex.instanse,                             // обьект плагина (this)
        reinterpret_cast<ContextHandle*>(&ctx),  // общая память
        reinterpret_cast<ParamsHandle*>(&params),// параметры шага
        &our_api                                 // наши хелперы
    );

    // Показываем, что вернул плагин
    std::cout << "[Engine] Результат: " << (res.str_val ? res.str_val : "(null)") << "\n";

    // Если есть save_as и результат — строка → сохраняем в context
    if (!save_as.empty() && res.type == 1 && res.str_val) {
        ctx.data[save_as] = res.str_val;
        std::cout << "[Engine] Сохранено в context: " << save_as << " = " << res.str_val << "\n";
    }

    // Шаг выполнен успешно
    return true;
}

// Главный метод — запуск всей задачи
bool Engine::run(const std::string& config_path) {
    // Парсим конфиг
    json config = parseConfig(config_path);
    if (config.is_null()) return false;

    // Создаём пустой context (общая память между шагами)
    Context ctx;
    // Проходим по всем шагам в конфиге
    for (const auto& step : config["steps"]) {
        // Выполняем шаг
        if (!executeStep(step, ctx)) {
            std::cerr << "[Engine] Ошибка выполнения шага\n";
            return false;
        }
    }
    // Если дошли сюда — всё хорошо
    std::cout << "[Engine] Задача завершена успешно\n";
    return true;
}
