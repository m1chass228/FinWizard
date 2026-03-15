#ifndef LIST_ALGORITHMS_H
#define LIST_ALGORITHMS_H

#endif // LIST_ALGORITHMS_H

// list_algorithms.h
#pragma once

#include <QObject>
#include <QListWidgetItem>

class MainWindow; // вперед объявляем MainWindow, чтобы не тянуть ui_mainwindow.h

class AlgoUI : public QObject
{
    Q_OBJECT

public:
    explicit AlgoUI(MainWindow* mainWin);

public slots:
    void loadAlgorithmsIntoUI();
    void onAlgorithmSelected();
    void onRunClicked();

private:
    MainWindow* m_main; // ссылка на MainWindow, чтобы обращаться к ui и методам
};
