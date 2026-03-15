// src/gui/main.cpp
#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QStyleFactory>
#include <finwizard/pluginmanager.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QCoreApplication::setOrganizationName("FinWizard");
    QCoreApplication::setApplicationName("FinWizardGui");

    qDebug() << "FinWizard dll запущен!";

    // 1. Создаем менеджер плагинов
    // Предположим, у него есть стандартный конструктор или метод получения инстанса
    PluginManager* manager = new PluginManager();

    // 2. Передаем этот менеджер в окно
    MainWindow window(manager);
    window.show();

    return app.exec();
}
