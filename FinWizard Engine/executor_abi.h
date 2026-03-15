// executor_abi.h
// Этот файл включают И движок, И все плагины
// Он НЕ должен содержать ничего, что зависит от конкретного компилятора или STL
// Только чистый C-стиль

#ifndef EXECUTOR_ABI_H          // Эта строчка + следующие две — защита от повторного включения файла
#define EXECUTOR_ABI_H          // Если файл уже включён — компилятор пропустит всё до #endif

#ifdef __cplusplus              // Если компилируем как C++ (а не как чистый C)
extern "C" {                    // Тогда говорим: "не порти имена функций, оставь их как в C"
#endif                          // Это очень важно для dlopen / LoadLibrary

// ────────────────────────────────────────────────
// Версия договора. Если мы изменим структуру — повысим число
// Плагин и движок проверяют, что версии совпадают
#define EXECUTOR_ABI_VERSION 1

// ────────────────────────────────────────────────
// "Непрозрачные" типы — это просто имена для указателей
// Движок видит ExecutorHandle*, но НЕ знает, что внутри
// Плагин знает, что это его собственный класс/структура
typedef struct ExecutorHandle      ExecutorHandle;
typedef struct ContextHandle       ContextHandle;
typedef struct ParamsHandle        ParamsHandle;

// ────────────────────────────────────────────────
// Это структура, которую плагин возвращает после выполнения шага
// Тип — чтобы понять, что внутри (строка, число, байты и т.д.)
typedef struct {
    int         type;           // 0 = ничего, 1 = строка, 2 = целое число, 3 = дробное, 4 = массив байт
    const char* str_val;        // если type == 1 — здесь указатель на строку (плагин сам управляет памятью)
    long long   int_val;        // если type == 2 — здесь целое число
    double      double_val;     // если type == 3
    const void* binary_ptr;     // если type == 4 — указатель на байты
    size_t      binary_size;    // и их размер
} ExecutorResult;

// ────────────────────────────────────────────────
// Хелперы — функции, которые ДВИЖОК даёт плагину
// Плагин через них читает параметры и пишет в общую память (context)
typedef struct {
    // Функция "дай мне строку по ключу из params"
    // Возвращает 1 = нашёл, 0 = нет такого ключа
    // out_str — указатель на строку (живёт до конца вызова execute!)
    int  (*get_param_str)(
        ParamsHandle* params,
        const char*   key,
        const char**  out_str
    );

    // Функция "запиши строку в context по ключу"
    // Движок сам скопирует строку, плагину не надо беспокоиться о памяти
    void (*set_context_str)(
        ContextHandle* ctx,
        const char*    key,
        const char*    value
    );

    // Позже добавим сюда:
    // get_param_int, set_context_int, get_param_binary и т.д.
} ExecutorApi;

// ────────────────────────────────────────────────
// Таблица функций, которую плагин отдаёт движку
// Это и есть те самые "телефоны"
typedef struct {
    // Самая главная функция — выполнение шага
    ExecutorResult (*execute)(
        ExecutorHandle* self,           // указатель на объект плагина (this)
        ContextHandle*  ctx,            // общая память между шагами
        ParamsHandle*   params,         // параметры именно этого шага
        const ExecutorApi* api          // кнопки, которые дал движок
    );

    // Функция очистки — когда движок скажет "хватит, удали себя"
    void (*destroy)(ExecutorHandle* self);

    // Версия договора — движок проверит, что она совпадает с EXECUTOR_ABI_VERSION
    int abi_version;
} ExecutorVTable;

// ────────────────────────────────────────────────
// Единственная функция, которую движок ищет в .so / .dll
// Имя должно быть точно "CreateExecutor" (extern "C" гарантирует это)
typedef ExecutorHandle* (*CreateExecutorFn)(
    const char*          executor_name,     // например "fs.copy" или "python.run"
    const ExecutorVTable** out_vtable,      // плагин сюда положит адрес своей таблицы
    const ExecutorApi*   provided_api       // движок передаёт сюда свои хелперы
);

#ifdef __cplusplus
}                               // закрываем extern "C"
#endif

#endif // EXECUTOR_ABI_H        // конец защиты от повторного включения
