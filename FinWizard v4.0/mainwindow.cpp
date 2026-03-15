#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "algorithmregistry.h"
#include "utils.h"
#include "logger.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadAlgorithmsIntoUI();

    // start button
    connect(ui->btnRun, &QPushButton::clicked, this, &MainWindow::onRunClicked);
    // list clicks
    connect(ui->listAlgorithms, &QListWidget::itemSelectionChanged, this, &MainWindow::onAlgorithmSelected);
    // list files double clicks
    connect(ui->list_files, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (!item) return;
        QString filePath = item->data(Qt::UserRole).toString();
        if (filePath.isEmpty()) return;
        // Открываем файл в приложении по умолчанию на системе
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    });
    // action bar
    connect(ui->actionAddFile, &QAction::triggered, this, &MainWindow::onAddFile);
    connect(ui->actionRemoveFile, &QAction::triggered, this, &MainWindow::onRemoveFile);
    connect(ui->actionClearAll, &QAction::triggered, this, &MainWindow::onClearAll);
    connect(ui->actionOpenDir, &QAction::triggered, this, &MainWindow::onOpenDir);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// algorithms into ui
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

// Algorithms clicks
void MainWindow::onAlgorithmSelected()
{
    ui->list_files->clear();

    QListWidgetItem *item = ui->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) return;

    int algoNum = item->data(Qt::UserRole).toInt();             // достаем номер алгоритма
    auto folder = AlgorithmRegistry::get_folder(algoNum);       // достаем папку алгоритма
    fs::path algo_dir = dir_on_desktop / folder.toStdString();  // достаем папку с исходниками

    if (folder.isEmpty()) {
        ui->lblStatus->setText("Папка не найдена!");
        return;
    }
    for (const auto& entry : xlsx_in_path(algo_dir)) {
        QString fullPath = QString::fromStdString(entry.string());
        QString fileName = QString::fromStdString(entry);

        QListWidgetItem *fileItem = new QListWidgetItem(fileName);
        fileItem->setData(Qt::UserRole, fullPath);

        ui->list_files->addItem(fileItem);
    }
}

// Run button
void MainWindow::onRunClicked()
{
    QListWidgetItem *item = ui->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) {
        ui->lblStatus->setText("Выберите алгоритм!");           // проверка выбора
        Logger::info("Выберите алгоритм!");
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
    Logger::info("Выполнение.....");

    // запуск
    func();

    ui->lblStatus->setText("Готово!");
    Logger::success("Готово!");
}

// action buttons
void MainWindow::onAddFile()
{
    QListWidgetItem *algoItem = ui->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString destDirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QDir destDir(destDirPath);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Добавить Excel-файлы",
        QDir::homePath(),
        "Excel (*.xlsx)"
        );

    for (const QString &path : files) {
        QFileInfo info(path);
        QString destPath = destDir.filePath(info.fileName());

        // если файл уже существует
        if (QFile::exists(destPath)) {
            QMessageBox::StandardButton reply =
                QMessageBox::question(
                    this,
                    "Файл уже существует",
                    QString("Файл \"%1\" уже есть.\nЗаменить его?")
                        .arg(info.fileName()),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
                    );
            if (reply == QMessageBox::Cancel) {
                return; // отменяем добавление
            }
            if (reply == QMessageBox::No) {
                continue; // пропускаем этот файл
            }
            QFile::remove(destPath); // Yes - заменяем
        }
        // копируем
        if (!QFile::copy(path, destPath)) {
            QMessageBox::warning(
                this,
                "Ошибка",
                QString("Не удалось скопировать файл:\n%1").arg(info.fileName())
                );
            continue;
        }
        // добавляем в список UI
        QListWidgetItem *item = new QListWidgetItem(info.fileName());
        item->setData(Qt::UserRole, destPath);
        ui->list_files->addItem(item);
        Logger::success(QString("Файл %1 успешно добавлен!").arg(info.fileName()));
    }
}
void MainWindow::onRemoveFile()
{
    QListWidgetItem *algoItem = ui->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString destDirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QDir destDir(destDirPath);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QList<QListWidgetItem*> items = ui->list_files->selectedItems();

    if (items.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите файл(ы) для удаления");
        return;
    }

    // подтверждение
    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Удаление файлов",
            QString("Удалить выбранные файлы (%1 шт.)?").arg(items.size()),
            QMessageBox::Yes | QMessageBox::No
            );

    if (reply != QMessageBox::Yes)
        return;

    // удаляем
    for (QListWidgetItem *item : items) {
        QString filePath = item->data(Qt::UserRole).toString();

        if (!filePath.isEmpty() && QFile::exists(filePath)) {
            if (!QFile::remove(filePath)) {
                QMessageBox::warning(
                    this,
                    "Ошибка",
                    QString("Не удалось удалить файл:\n%1")
                        .arg(QFileInfo(filePath).fileName())
                    );
                continue;
            }
        }
        // удаляем из списка UI
        delete ui->list_files->takeItem(ui->list_files->row(item));
        Logger::success(QString("Файл %1 успешно удален!").arg(QFileInfo(filePath).fileName()));
    }
}
void MainWindow::onClearAll()
{
    QListWidgetItem *algoItem = ui->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString dirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Очистка",
            "Удалить ВСЕ .xlsx файлы для этого алгоритма?",
            QMessageBox::Yes | QMessageBox::No
            );

    if (reply != QMessageBox::Yes)
        return;

    QDir dir(dirPath);
    QStringList files = dir.entryList(QStringList() << "*.xlsx", QDir::Files);

    for (const QString &file : files) {
        if (!QFile::remove(dir.filePath(file))) {
            QMessageBox::warning(
                this,
                "Ошибка",
                QString("Не удалось удалить файл:\n%1")
                    .arg(file)
                );
            continue;
        }
    }
    ui->list_files->clear();
    Logger::success("Все файлы алгоритма удалены");
}
void MainWindow::onOpenDir()
{
    QListWidgetItem *algoItem = ui->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString dirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;
    QDir dir(dirPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "Ошибка", "Папка алгоритма не существует");
        return;
    }
    // открыть в файловом менеджере
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
}
// console with logs
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
