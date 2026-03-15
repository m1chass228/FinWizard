#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void logMessage(const QString& message, const QString& color = "black");

private:
    // algo list
    void loadAlgorithmsIntoUI();
    // start butt
    void onRunClicked();
    // algo click
    void onAlgorithmSelected();

    // toolbar actions
    void onAddFile();
    void onRemoveFile();
    void onClearAll();
    void onOpenDir();

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
