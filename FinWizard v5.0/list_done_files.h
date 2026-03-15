#ifndef LIST_DONE_FILES_H
#define LIST_DONE_FILES_H

#endif // LIST_DONE_FILES_H

// list_done_files.h
#pragma once

#include <QObject>
#include <QListWidgetItem>

class MainWindow; // вперед объявляем MainWindow, чтобы не тянуть ui_mainwindow.h

class DoneUI : public QObject
{
    Q_OBJECT

public:
    explicit DoneUI(MainWindow* mainWin);

public slots:
    void loadDoneFilesIntoUI();
    void showOutputContextMenu(const QPoint &pos);


private:
    MainWindow* m_main; // ссылка на MainWindow, чтобы обращаться к ui и методам
};
