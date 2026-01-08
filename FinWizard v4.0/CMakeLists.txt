cmake_minimum_required(VERSION 3.16)

project(Raz_ZP_qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------ Find Qt ------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ------------------ Basic project sources ------------------
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    algorithmregistry.cpp
    algorithmregistry.h
    utils.h
    utils.cpp
    logger.h
    logger.cpp

)

# ------------------ Add algorithms via GLOB ------------------
file(GLOB_RECURSE ALG_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/**/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/algorithms/**/*.h"
    message(STATUS "ALG_SRC files:")
    foreach(f ${ALG_SRC})
        message(STATUS "  ${f}")
    endforeach()
)

# ------------------ Create executable ------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Raz_ZP_qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${ALG_SRC}
    )
else()
    add_executable(Raz_ZP_qt
        ${PROJECT_SOURCES}
        ${ALG_SRC}
    )
endif()

target_include_directories(Raz_ZP_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Raz_ZP_qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# ------------------ QXlsx ------------------
add_subdirectory(QXlsx/QXlsx ${CMAKE_BINARY_DIR}/QXlsx_build)
target_link_libraries(Raz_ZP_qt PRIVATE QXlsx::QXlsx)

# ------------------ Finalize for Qt6 ------------------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Raz_ZP_qt)
endif()
