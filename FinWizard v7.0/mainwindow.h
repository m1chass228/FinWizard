#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlistwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class AlgoUI;
class DoneUI;
class FilesUI;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // публичный метод для доступа к ui
    Ui::MainWindow* getUi() { return ui; }

    // публичный метод для доступа к doneUI
    DoneUI* getDoneUI() {return doneUI; }

    // публичный метод для доступа к filesUI
    FilesUI* getFilesUI() { return filesUI; }

    // публичный метод для доступа к filesUI
    AlgoUI* getAlgoUI() { return algoUI; }

    void logMessage(const QString& message, const QString& color = "black");

private:
    AlgoUI* algoUI;
    FilesUI* filesUI;
    DoneUI* doneUI;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
