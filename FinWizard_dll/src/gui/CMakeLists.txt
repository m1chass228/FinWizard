# Минимальная версия CMake для Qt6
cmake_minimum_required(VERSION 3.16)

# Название проекта
project(FinWizardGui LANGUAGES CXX)

# Включаем автоматическую обработку moc, uic, rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Стандарт C++17 (обязательно для Qt6)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим Qt6
find_package(Qt6 6.5 COMPONENTS Widgets REQUIRED)

# Исполняемый файл
add_executable(FinWizardGui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc         # <--- Иконка для окна (внутри программы)
    ${PLATFORM_SOURCES}   # <--- Иконка для .exe (подтянется только на Windows)
)

# Подключаем Qt6::Widgets
target_link_libraries(FinWizardGui
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        core          # <--- Просто линкуем наше ядро целиком!
        QuaZip::QuaZip
)

# Если у тебя есть свои include-пути (например для pluginmanager.h)
target_include_directories(FinWizardGui PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/include
)

# Копируем примеры конфигов в папку сборки после сборки
if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/examples")
    add_custom_command(TARGET FinWizardGui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/plugins/examples
                $<TARGET_FILE_DIR:FinWizardGui>/plugins
        COMMENT "Копируем примеры конфигов в папку сборки"
    )
endif()

# Для Linux — RPATH (чтобы находил библиотеки рядом с исполняемым файлом)
if(UNIX AND NOT APPLE)
    set_target_properties(FinWizardGui PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
