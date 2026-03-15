// core/Engine.h
#pragma once

#include "executor_abi.h"
#include "../third-party/json-3.12.0/single_include/nlohmann/json.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
    using LibHandle = HMODULE;
    #define DL_OPEN(path) LoadLibraryA(path)
    #define DL_SYM(h, name) GetProcAddress(h, name)
    #define DL_CLOSE(h) FreeLibrary(h)
    #define DL_ERROR() std::to_string(GetLastError())
#else 
    #include <dlfcn.h>
    using LibHandle = void*;
    #define DL_OPEN(path) dlopen(path, RTLD_LAZY)
    #define DL_SYM(h, name) dlsym(h, name)
    #define DL_CLOSE(h) dlclose(h)
    #define DL_ERROR() dlerror()
#endif

using json = nlohmann::json;

// Простая реализация Context и Params (пока только строки, потом расширим)
using StringMap = std::unordered_map<std::string, std::string>;

struct Context {
    StringMap data;
};

struct Params {
    StringMap data;
};

class Engine {
public:
    Engine();
    ~Engine();

    bool loadPlugin(const std::string& lib_path, const std::string& executor_name);

    bool run(const std::string& config_path);

private: 
    // Храним загруженные плагины
    struct LoadedExecutor {
        LibHandle handle = nullptr;
        ExecutorHandle* instanse = nullptr;
        const ExecutorVTable* vtable = nullptr;
    };

    std::unordered_map<std::string, LoadedExecutor> executors;

    // Хелперы для плагинов (реализация ниже)
    static int static_get_param_str(ParamsHandle* p, const char* k, const char** out);
    static void static_set_context_str(ContextHandle* c, const char* k, const char* v);

    static const ExecutorApi our_api;

    // Вспомогательные методы
    json parseConfig(const std::string& path);
    bool executeStep(const json& step, Context& ctx);

}