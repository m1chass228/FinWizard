#ifndef LOGGER_H
#define LOGGER_H

#endif // LOGGER_H

#pragma once

#include <QString>

// Глобальный логгер для всего приложения
class Logger
{
public:
    // Разные уровни логов с цветами
    static void info(const QString& message);
    static void success(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);

    // Обычный лог без цвета (если нужно)
    static void log(const QString& message, const QString& color = "black");

private:
    // Запрещаем создание экземпляров
    Logger() = delete;
};
