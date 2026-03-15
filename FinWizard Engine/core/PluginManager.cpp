#include <PluginManager.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "executor_abi.h"

bool PluginManager::load(const std::string &lib_path, const std::string &executor_name) {

    void* handle = nullptr;
// загружаем dll / so
#ifdef _WIN32
    handle = LoadLibraryA(lib_path.c_str());
    if (!handle) {
        std::cerr << "[PluginManager] LoadLibraryA failed: " << GetLastError() << "\n";
        return false;
    }
#else
    handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);  // RTLD_NOW — сразу все символы
    if (!handle) {
        std::cerr << "[PluginManager] dlopen failed: " << dlerror() << "\n";
        return false;
    }
#endif

    // ищем функцию с именем CreateExecutor
    CreateExecutorFn create = nullptr;
#ifdef _WIN32
    create = (CreateExecutorFn)GetProcAddress(handle, "CreateExecutor");
#else
    create = (CreateExecutorFn)dlsym(handle, "CreateExecutor");
#endif

    // проверяем, есть ли функция
    if (!create) {
        std::cerr << "[PluginManager] CreateExecutor не найдена в " << lib_path << "\n";

        // выгружаем библиотеку
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // если есть - вызываем фабрику плагина.
    // передаем имя, Адрес переменной vtable (плагин заполнит её), Наш ExecutorApi (хелперы для плагина)
    const ExecutorVTable* vtable = nullptr;
    ExecutorHandle* instance = create(executor_name.c_str(), &vtable, &our_api);

    if (!instance || !vtable) {
        std::cerr << "[PluginManager] CreateExecutor вернул nullptr для " << executor_name << "\n";
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // Проверяем, что плагин вернул нормальные указатели
    if (vtable->abi_version != EXECUTOR_ABI_VERSION) {
        std::cerr << "[PluginManager] Несовместимая версия ABI: " << vtable->abi_version << " != " << EXECUTOR_ABI_VERSION << "\n";
        vtable->destroy(instance);
#ifdef _WIN32
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return false;
    }

    // сохраняем в map
    loaded[executor_name] = {handle, instance, vtable};
    std::cout << "[PluginManager] Успешно загружен: " << executor_name << " из " << lib_path << "\n";
    return true;
}
