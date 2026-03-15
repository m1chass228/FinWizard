#ifndef LIST_FILES_H
#define LIST_FILES_H

#endif // LIST_FILES_H

// list_files.h
#pragma once

#include <QObject>
#include <QListWidgetItem>

class MainWindow; // вперед объявляем MainWindow, чтобы не тянуть ui_mainwindow.h

class FilesUI : public QObject
{
    Q_OBJECT

public:
    explicit FilesUI(MainWindow* mainWin);

public slots:
    void onAddFile();
    void onRemoveFile();
    void onClearAll();
    void onOpenDir();
    void showOutputContextMenu(const QPoint &pos);

private:
    MainWindow* m_main; // ссылка на MainWindow, чтобы обращаться к ui и методам
};
