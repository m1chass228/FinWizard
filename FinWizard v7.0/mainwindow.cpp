// mainwindow.cpp
#include "list_algorithms.h"
#include "list_done_files.h"
#include "list_files.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , doneUI(new DoneUI(this))
    , filesUI(new FilesUI(this))
    , algoUI(new AlgoUI(this))
{
    ui->setupUi(this);

    // инициализация списка с алгоритмами
    algoUI->loadAlgorithmsIntoUI();

    // инициализация списка с готовыми файлами
    doneUI->loadDoneFilesIntoUI();

    // контекстное меню и параметры отображения для списка готовых файлов
    ui->listOutputFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listOutputFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listOutputFiles->setViewMode(QListView::IconMode);
    ui->listOutputFiles->setResizeMode(QListView::Adjust);
    ui->listOutputFiles->setMovement(QListView::Static);
    ui->listOutputFiles->setSpacing(8);
    ui->listOutputFiles->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->listOutputFiles->setWrapping(true);
    connect(ui->listOutputFiles, &QListWidget::customContextMenuRequested, doneUI, &DoneUI::showOutputContextMenu);

    // кнопка запуска
    connect(ui->btnRun, &QPushButton::clicked, algoUI, &AlgoUI::onRunClicked);

    // выбор алгоритма
    connect(ui->listAlgorithms, &QListWidget::itemSelectionChanged, algoUI, &AlgoUI::onAlgorithmSelected);

    // двойной клик по файлу в списке с файлами
    connect(ui->list_files, &QListWidget::itemDoubleClicked, filesUI, [this](QListWidgetItem* item) {
        if (!item) return;
        QString filePath = item->data(Qt::UserRole).toString();
        if (filePath.isEmpty()) return;
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    });

    // контекстное меню и параметры отображения для списка файлов
    ui->list_files->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->list_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->list_files->setViewMode(QListView::IconMode);
    ui->list_files->setResizeMode(QListView::Adjust);
    ui->list_files->setMovement(QListView::Static);
    ui->list_files->setSpacing(8);
    ui->list_files->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->list_files->setWrapping(true);
    connect(ui->list_files, &QListWidget::customContextMenuRequested, filesUI, &FilesUI::showOutputContextMenu);

    // action bar
    connect(ui->actionAddFile, &QAction::triggered, filesUI, &FilesUI::onAddFile);
    connect(ui->actionRemoveFile, &QAction::triggered, filesUI, &FilesUI::onRemoveFile);
    connect(ui->actionClearAll, &QAction::triggered, filesUI, &FilesUI::onClearAll);
    connect(ui->actionOpenDir, &QAction::triggered, filesUI, &FilesUI::onOpenDir);
}

MainWindow::~MainWindow()
{
    delete ui;
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
