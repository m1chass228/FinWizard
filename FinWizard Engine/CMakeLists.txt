cmake_minimum_required(VERSION 3.15)
project(FinWizardEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Подключаем nlohmann/json (single_include)
include_directories(third-party/json-3.12.0/single_include)

# Для dlopen на Linux
if(UNIX AND NOT APPLE)
    link_libraries(dl)
endif()

# Все исходники core (твои реальные файлы)
add_library(core STATIC
    core/Context.cpp
    core/Engine.cpp
    core/Scheduler.cpp        # ← если переименовал Sheduler → Scheduler
    core/Task.cpp
    core/Value.cpp
    core/Parser.cpp
    core/PluginManager.cpp
)

target_include_directories(core PUBLIC core .)

# Добавляем .h-файлы только для Qt Creator (не влияет на сборку)
set(HEADER_FILES
    core/Context.h
    core/Engine.h
    core/Scheduler.h
    core/Task.h
    core/Value.h
    core/ExecutorAPI.h
    core/Parser.h
    core/PluginManager.h
    executor_abi.h
)

# Исполняемый файл (main.cpp в корне проекта)
add_executable(engine main.cpp)
target_link_libraries(engine PRIVATE core)

# Чтобы .h-файлы были видны в Qt Creator
target_sources(engine PRIVATE ${HEADER_FILES})
