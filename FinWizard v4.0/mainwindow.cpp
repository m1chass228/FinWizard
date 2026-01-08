#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "algorithmregistry.h"
#include "utils.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadAlgorithmsIntoUI();

    // подключаем кнопку
    connect(ui->btnRun, &QPushButton::clicked, this, &MainWindow::onRunClicked);
    connect(ui->listAlgorithms, &QListWidget::itemSelectionChanged, this, &MainWindow::onAlgorithmSelected);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadAlgorithmsIntoUI()
{
    ui->listAlgorithms->clear();

    auto &map = AlgorithmRegistry::list();  // получаем ссылку на карту всех алгоритмов
    for (auto &[num, info] : map) {         // идем по всем зарегистрированным алгоритмам
        QListWidgetItem *item =             // создаем новый элемент для QListWidget
            new QListWidgetItem(QString("%1 - %2").arg(num).arg(info.name));    // num - номер алгоритма,
        // info - структура AlgorithmInfo в которой
        // name, folder, func
        item->setData(Qt::UserRole, num);   // сохраняем ключ алгоритма в item
        ui->listAlgorithms->addItem(item);  // выводим
    }
}
void MainWindow::onAlgorithmSelected()
{


    QListWidgetItem *item = ui->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) {
        ui->lblStatus->setText("Выберите алгоритм!");           // проверка выбора
        return;
    }
    int algoNum = item->data(Qt::UserRole).toInt();             // достаем номер алгоритма
    auto folder = AlgorithmRegistry::get_folder(algoNum);       // достаем папку алгоритма

    ui->list_files->clear();

    if (folder.isEmpty()) {
        ui->lblStatus->setText("Папка не найдена!");
        return;
    }
    fs::path algo_dir = dir_on_desktop / std::string(folder.toUtf8().constData());
    for (auto entry : xlsx_in_path(algo_dir)) {
        ui->list_files->addItem(QString::fromStdString(entry.filename().string()));
    }
}

void MainWindow::onRunClicked()
{
    QListWidgetItem *item = ui->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) {
        ui->lblStatus->setText("Выберите алгоритм!");           // проверка выбора
        return;
    }
    int algoNum = item->data(Qt::UserRole).toInt();             // достаем номер алгоритма

    rm_rf_xlsx(dir_on_desktop);

    auto func = AlgorithmRegistry::get(algoNum);                // берем функцию по номеру
    if (!func) {
        ui->lblStatus->setText("Ошибка! функция не найдена!");
        return;
    }
    ui->lblStatus->setText("Выполнение......");

    // запуск
    func();

    ui->lblStatus->setText("Готово!");
}

void MainWindow::logMessage(const QString& message, const QString& color)
{
    if (!ui || !ui->konsoleText) {
        qDebug().noquote() << "[LOG]" << message;
        return;
    }

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString escaped = message.toHtmlEscaped();
    QString colored = QString("<span style='color:%1;'>%2</span>").arg(color, escaped);

    QString line = QString("<b>[%1]</b> %2").arg(timestamp, colored);

    ui->konsoleText->append(line);
    ui->konsoleText->repaint(); // сразу обновить
    QApplication::processEvents(); // обработать события GUI

    // Автопрокрутка
    QScrollBar *sb = ui->konsoleText->verticalScrollBar();
    if (sb) {
        sb->setValue(sb->maximum());
    }
}
