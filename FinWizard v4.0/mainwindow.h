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
    void loadAlgorithmsIntoUI();
    void onRunClicked();
    void onAlgorithmSelected();
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
