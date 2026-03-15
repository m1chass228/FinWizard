// src/gui/mainwindow.cpp
#include "mainwindow.h"
#include "finwizard/pluginmanager.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QCheckBox>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QClipboard>
#include <QMimeData>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/JlCompress.h>

MainWindow::MainWindow(PluginManager *pluginManager, QWidget *parent)
    : QMainWindow(parent)
    , m_pluginManager(pluginManager)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("FinWizard - Мастер обработки данных");
    setWindowIcon(QIcon(":/res/icon.png"));

    if (!m_pluginManager) {
        qCritical() << "PluginManager is null!";
        return;
    }

    // --- 1. ПУТИ И НАСТРОЙКИ ---
    QSettings settings("FinWizard", "Settings");
    QString configsPath = settings.value("cache/path",
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/FinWizard/plugins-cache").toString();
    QString inputPath = settings.value("inputFolder",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FinWizard_Input").toString();

    QDir().mkpath(configsPath);
    QDir().mkpath(inputPath);

    // --- 2. ИНИЦИАЛИЗАЦИЯ МЕНЕДЖЕРА ---
    m_pluginManager->setCacheBasePath(configsPath);
    m_pluginManager->refreshPlugins();

    // --- 3. WATCHER И ТАЙМЕР ---
    m_watchdogTimer = new QTimer(this);
    m_watchdogTimer->setSingleShot(true);
    m_watchdogTimer->setInterval(500);

    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(configsPath);
    m_watcher->addPath(inputPath);

    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &MainWindow::onDirectoryChanged);

    // Когда таймер дотикает (прошло 500мс после последнего изменения в папках)
    connect(m_watchdogTimer, &QTimer::timeout, this, [this]() {
        if (m_needsPluginUpdate) {
            m_pluginManager->refreshPlugins();
            updateConfigList();
            m_needsPluginUpdate = false; // сбрасываем флаг
        }

        if (m_needsXlsxUpdate) {
            updateXlsxList();
            m_needsXlsxUpdate = false; // сбрасываем флаг
        }
    });

    // --- 4. СОБЫТИЯ И ИНТЕРФЕЙС ---
    connect(ui->configComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onConfigSelected);
    connect(ui->browseXlsxButton, &QPushButton::clicked, this, &MainWindow::onBrowseXlsxClicked);
    connect(ui->openFolderButton, &QPushButton::clicked, this, &MainWindow::onOpenFolderClicked);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    // --- 5. КОНТЕКСТНОЕ МЕНЮ ---
    ui->xlsxList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->xlsxList, &QListWidget::customContextMenuRequested, this, &MainWindow::showXlsxContextMenu);

    // Первая отрисовка
    updateConfigList();
    updateXlsxList();

    logMessage("Программа запущена. Конфиги загружены из кэша.", false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showXlsxContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->xlsxList->itemAt(pos);
    QMenu contextMenu(this);

    if (item) {
        QAction *copyAction = contextMenu.addAction("Копировать имя");
        QAction *openAction = contextMenu.addAction("Открыть файл");
        contextMenu.addSeparator();
        QAction *deleteAction = contextMenu.addAction("Удалить файл");

        QAction *selectedAction = contextMenu.exec(ui->xlsxList->mapToGlobal(pos));

        if (selectedAction == copyAction) {
            QApplication::clipboard()->setText(item->text());
            logMessage("Имя файла скопировано в буфер", false);
        }
        else if (selectedAction == openAction) {
            QSettings settings("FinWizard", "Settings");
            QString inputFolder = settings.value("inputFolder").toString();
            QString fullPath = QDir(inputFolder).absoluteFilePath(item->text());
            QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
        }
        else if (selectedAction == deleteAction) {
            auto res = QMessageBox::question(this, "Удаление", "Удалить файл " + item->text() + "?", QMessageBox::Yes | QMessageBox::No);
            if (res == QMessageBox::Yes) {
                QSettings settings("FinWizard", "Settings");
                QString inputFolder = settings.value("inputFolder").toString();
                QFile file(QDir(inputFolder).absoluteFilePath(item->text()));

                if (file.remove()) {
                    logMessage("Файл удален: " + item->text(), false);
                    updateXlsxList();
                } else {
                    logMessage("Не удалось удалить файл: " + file.errorString(), true);
                }
            }
        }
    }
    else {
        QAction *addFilesAction = contextMenu.addAction("Добавить файлы...");
        QAction *refreshAction = contextMenu.addAction("Обновить список");

        QAction *selectedAction = contextMenu.exec(ui->xlsxList->mapToGlobal(pos));

        if (selectedAction == addFilesAction) {
            onBrowseXlsxClicked();
        } else if (selectedAction == refreshAction) {
            updateXlsxList();
        }
    }
}

void MainWindow::onStartClicked()
{
    QSettings settings("FinWizard", "Settings");
    QDir inputDir(settings.value("inputFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString());

    // 1. Проверяем, выбран ли конфиг
    if (ui->configComboBox->currentIndex() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите конфиг из списка");
        return;
    }

    int id = ui->configComboBox->currentData().toInt();

    // 2. Получаем input файлы
    if (!inputDir.exists()) {
        logMessage(QString("Папка input не существует: %1").arg(inputDir.absolutePath()), true);
        return;
    }

    QStringList inputXlsxFiles = inputDir.entryList({"*.xlsx", "*.xls"}, QDir::Files | QDir::NoDotAndDotDot);
    if (inputXlsxFiles.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "В папке input нет XLSX-файлов");
        logMessage("СТАРТ: файлы xlsx не найдены", true);
        return;
    }

    // 3. Собираем параметры
    QStringList fullPaths;
    for (const QString &fileName : inputXlsxFiles) {
        fullPaths << inputDir.absoluteFilePath(fileName);
    }

    QString outputFolder = settings.value("outputFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FinWizard_Results").toString();
    QDir outputDir(outputFolder);
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать папку результатов:\n" + outputFolder);
        return;
    }

    QString desiredFileName = ui->fileName->text().trimmed();

    if (desiredFileName.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Введите название файла!");
        return;
    }

    else if (!desiredFileName.endsWith(".xlsx", Qt::CaseInsensitive)) {
        desiredFileName += ".xlsx";
    }

    // Безопасно склеиваем папку и имя файла (Qt сам поставит правильный слэш)
    QString outputFile = outputDir.absoluteFilePath(desiredFileName);

    // Проверяем
    QFile file(outputFile);
    if (file.exists()) {
        QMessageBox::critical(this, "Ошибка", "Файл с таким именем уже существует!");
        return;
    }

    QVariantMap params;
    params["xlsxFiles"] = fullPaths;
    params["outputFolder"] = outputFolder;
    params["outputFile"] = outputFile;
    params["desiredFileName"] = desiredFileName;

    // 4. ЗАПУСК! (Менеджер сам разберется с загрузкой в память)
    logMessage("Запуск конфига ID " + QString::number(id) + "...", false);
    QVariantMap result = m_pluginManager->runConfig(id, params);

    // 5. Обработка результата
    if (result.value("success").toBool()) {
        QString msg = result.value("message").toString();
        QString outPath = result.value("outputPath").toString();

        logMessage("Успех: " + msg, false);

        if (!outPath.isEmpty()) {
            logMessage("Результат сохранён: " + outPath, false);

            if (settings.value("autoOpenResultFolder", true).toBool()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(outPath).absolutePath()));
            }
        }

        // 6. Очищаем поле для ввода имени ТОЛЬКО при успехе!
        ui->fileName->clear();
    } else {
        QString error = result.value("error").toString();
        logMessage("Ошибка: " + error, true);
        QMessageBox::critical(this, "Ошибка выполнения", error);
    }
    // 6. Очищаем поле для ввода имени
}


void MainWindow::onBrowseXlsxClicked()
{
    QSettings settings("FinWizard", "Settings");
    QString inputFolder = settings.value("inputFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FinWizard_Input").toString();

    QDir inputDir(inputFolder);
    if (!inputDir.exists() && !inputDir.mkpath(".")) return;

    QStringList selectedFiles = QFileDialog::getOpenFileNames(
        this, "Выберите XLSX-файлы или ZIP-архивы",
        settings.value("lastInputFolder").toString(),
        "Поддерживаемые файлы (*.xlsx *.xls *.zip)"
    );

    if (selectedFiles.isEmpty()) return;

    settings.setValue("lastInputFolder", QFileInfo(selectedFiles.first()).absolutePath());

    for (const QString &filePath : selectedFiles) {
        QFileInfo fi(filePath);
        QString ext = fi.suffix().toLower();

        if (ext == "xlsx" || ext == "xls") {
            QString dest = inputFolder + "/" + fi.fileName();
            if (QFile::copy(filePath, dest)) {
                logMessage("Скопирован: " + fi.fileName(), false);
            } else {
                logMessage("Ошибка копирования: " + fi.fileName(), true);
            }
        }
        else if (ext == "zip") {
            QTemporaryDir tempDir;
            QStringList extracted = JlCompress::extractDir(filePath, tempDir.path());

            QDir extractDir(tempDir.path());
            QStringList xlsxFiles = extractDir.entryList({"*.xlsx", "*.xls"}, QDir::Files);

            for (const QString &xlsxName : xlsxFiles) {
                QString dest = inputFolder + "/" + xlsxName;
                if (QFile::copy(extractDir.absoluteFilePath(xlsxName), dest)) {
                    logMessage("Скопирован из ZIP: " + xlsxName, false);
                }
            }
        }
    }
    updateXlsxList();
}

void MainWindow::onSettingsClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Настройки");
    dlg.resize(650, 420);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dlg);
    QSettings settings("FinWizard", "Settings");

    QCheckBox *autoOpenCheck = new QCheckBox("Автоматически открывать папку с результатами", &dlg);
    autoOpenCheck->setChecked(settings.value("autoOpenResultFolder", true).toBool());
    mainLayout->addWidget(autoOpenCheck);

    auto createPathRow = [&](const QString &label, const QString &key, const QString &def) {
        QHBoxLayout *row = new QHBoxLayout();
        row->addWidget(new QLabel(label, &dlg));
        QLineEdit *edit = new QLineEdit(settings.value(key, def).toString(), &dlg);
        QPushButton *btn = new QPushButton("Обзор", &dlg);
        connect(btn, &QPushButton::clicked, [&dlg, edit]() {
            QString path = QFileDialog::getExistingDirectory(&dlg, "Выберите папку", edit->text());
            if (!path.isEmpty()) edit->setText(path);
        });
        row->addWidget(edit);
        row->addWidget(btn);
        mainLayout->addLayout(row);
        return edit;
    };

    QLineEdit *resultEdit = createPathRow("Папка для результатов:", "outputFolder", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FinWizard_Results");
    QLineEdit *inputEdit = createPathRow("Папка для входных файлов:", "inputFolder", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/FinWizard");
    QLineEdit *cacheEdit = createPathRow("Папка кэша плагинов:", "cache/path", QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/FinWizard/plugins-cache");

    QPushButton *saveBtn = new QPushButton("Сохранить", &dlg);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        settings.setValue("autoOpenResultFolder", autoOpenCheck->isChecked());
        settings.setValue("outputFolder", resultEdit->text());
        settings.setValue("inputFolder", inputEdit->text());
        settings.setValue("cache/path", cacheEdit->text());

        m_pluginManager->setCacheBasePath(cacheEdit->text());

        // Обновляем пути в вотчере
        if (!m_watcher->directories().isEmpty()) {
            m_watcher->removePaths(m_watcher->directories());
        }
        m_watcher->addPath(inputEdit->text());
        m_watcher->addPath(cacheEdit->text());

        updateConfigList();
        updateXlsxList();
        logMessage("Настройки сохранены", false);
        dlg.accept();
    });

    QPushButton *cancelBtn = new QPushButton("Отмена", &dlg);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    dlg.exec();
}

void MainWindow::onDirectoryChanged(const QString &path)
{
    QSettings settings("FinWizard", "Settings");

    // Получаем текущие пути из настроек (очищаем их от лишних слешей для точного сравнения)
    QString configsPath = QDir::cleanPath(settings.value("cache/path").toString());
    QString inputPath = QDir::cleanPath(settings.value("inputFolder").toString());
    QString changedPath = QDir::cleanPath(path);

    if (changedPath == configsPath) {
        m_needsPluginUpdate = true;
    }
    else if (changedPath == inputPath) {
        m_needsXlsxUpdate = true;
    }

    // Запускаем/перезапускаем таймер.
    // Пока файлы копируются (идет спам событий), таймер будет постоянно сбрасываться.
    // Как только копирование завершится, таймер отсчитает 500мс и обновит UI.
    m_watchdogTimer->start();
}

void MainWindow::updateXlsxList()
{
    ui->xlsxList->clear();
    QSettings settings("FinWizard", "Settings");
    QDir inputPath(settings.value("inputFolder").toString());
    ui->xlsxList->addItems(inputPath.entryList({"*.xlsx"}, QDir::Files));
}

void MainWindow::updateConfigList()
{
    int savedId = ui->configComboBox->currentData().toInt();

    ui->configComboBox->blockSignals(true);
    ui->configComboBox->clear();
    ui->configComboBox->addItem("Выберите конфиг...", 0);

    QStringList names = m_pluginManager->getDisplayNames();
    QList<int> ids = m_pluginManager->getAllConfigIds();

    int indexToSelect = 0;
    for (int i = 0; i < names.size(); ++i) {
        ui->configComboBox->addItem(names[i], ids[i]);
        if (ids[i] == savedId) indexToSelect = ui->configComboBox->count() - 1;
    }

    ui->configComboBox->addItem("➕ Добавить конфиг...", -999);
    ui->configComboBox->setCurrentIndex(indexToSelect);
    ui->configComboBox->blockSignals(false);
}

void MainWindow::onConfigSelected(int index)
{
    int selectedId = ui->configComboBox->itemData(index).toInt();

    if (selectedId == -999) {
        QMetaObject::invokeMethod(this, "onAddConfigClicked", Qt::QueuedConnection);
        ui->configComboBox->setCurrentIndex(0);
        return;
    }

    if (selectedId <= 0) {
        ui->startButton->setEnabled(false);
        return;
    }

    // Мы больше не тащим IConfig и CachedConfig сюда. Берем имя прямо из ComboBox!
    QString configName = ui->configComboBox->itemText(index);
    logMessage(QString("Выбран конфиг: %1").arg(configName), false);

    updateConfigPreview(selectedId);
    ui->startButton->setEnabled(true);
}

void MainWindow::updateConfigPreview(int configId)
{
    ui->configIcon->clear();
    ui->configName->setText("Не выбрано");
    ui->configDescription->clear();

    QMap<QString, QString> preview = m_pluginManager->getConfigPreview(configId);
    if (preview.isEmpty()) return;

    if (!preview["name"].isEmpty()) ui->configName->setText(preview["name"]);
    if (!preview["description"].isEmpty()) ui->configDescription->setText(preview["description"]);

    if (!preview["icon"].isEmpty()) {
        QPixmap pixmap(preview["icon"]);
        if (!pixmap.isNull()) {
            ui->configIcon->setPixmap(pixmap.scaled(ui->configIcon->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->configIcon->setScaledContents(true);
        }
    }
}

void MainWindow::onAddConfigClicked()
{
    QString zipPath = QFileDialog::getOpenFileName(this, "Выберите ZIP-файл", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "ZIP (*.zip)");
    if (zipPath.isEmpty()) return;

    int newId = m_pluginManager->addConfigFromZip(zipPath);
    if (newId != -1) {
        updateConfigList();
        int newIndex = ui->configComboBox->findData(newId);
        if (newIndex != -1) ui->configComboBox->setCurrentIndex(newIndex);
    }
}

void MainWindow::logMessage(const QString &message, bool isError)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString prefix = isError ? "[ОШИБКА]" : "[INFO]";
    QString color = isError ? "red" : "lime";
    ui->logTextEdit->append(QString("%1 %2 <font color=\"%3\"><b>%4</b></font>").arg(time, prefix, color, message.toHtmlEscaped()));
    ui->logTextEdit->ensureCursorVisible();
}

void MainWindow::onOpenFolderClicked()
{
    QSettings settings("FinWizard", "Settings");
    QDesktopServices::openUrl(QUrl::fromLocalFile(settings.value("cache/path").toString()));
}
