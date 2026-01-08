// Logger.cpp
#include "logger.h"
#include "mainwindow.h"       // ← нужен для MainWindow
#include <QApplication>
#include <QDebug>

void Logger::log(const QString& message, const QString& color)
{
    MainWindow* mw = nullptr;
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        mw = qobject_cast<MainWindow*>(widget);
        if (mw) break;
    }

    if (mw) {
        mw->logMessage(message, color);  // ← используем публичный метод
    } else {
        // Если окна ещё нет — в консоль разработчика
        qDebug().noquote() << "[LOG]" << message;
    }
}

void Logger::info(const QString& message)    { log(message, "#0066cc"); }
void Logger::success(const QString& message){ log("✓ " + message, "green"); }
void Logger::warning(const QString& message){ log("⚠ " + message, "orange"); }
void Logger::error(const QString& message)  { log("✗ " + message, "red"); }
