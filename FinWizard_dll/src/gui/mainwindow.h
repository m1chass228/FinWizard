// src/gui/mainwindow.h
#pragma once

#include <QMainWindow>
#include "ui_mainwindow.h"  // ← это важно! Qt Designer генерирует этот файл
#include <QFileSystemWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // ← ОБЯЗАТЕЛЬНО
QT_END_NAMESPACE

class PluginManager;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(PluginManager *pluginManager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateConfigList();                  // обновить список конфигов в configComboBox
    void updateXlsxList();                    // обновить список файлов в QlistWidget
    void onConfigSelected(int index);         // при выборе конфига в выпадающем списке
    void onBrowseXlsxClicked();               // кнопка "Обзор" для XLSX
    void onOpenFolderClicked();               // кнопка "Открыть папку с конфигами"
    void onAddConfigClicked();                // добавить конфиг
    void onStartClicked();                    // большая кнопка "СТАРТ"
    void onSettingsClicked();                 // кнопка "Настройки" (пока заглушка)
    void onDirectoryChanged(const QString &path); // если папки меняются
    void logMessage(const QString &msg, bool isError = false);
    void updateConfigPreview(int configId);   // картинка
    void showXlsxContextMenu(const QPoint &pos); // контекстное меню

private:
    PluginManager *m_pluginManager;
    QString m_currentXlsxPath;                // путь к выбранному XLSX-файлу
    QFileSystemWatcher *m_watcher;
    QTimer *m_watchdogTimer;
    Ui::MainWindow *ui;

    bool m_needsPluginUpdate = false;
    bool m_needsXlsxUpdate = false;
};
