//list_files.cpp
#include "list_files.h"
#include "algorithmregistry.h"
#include "utils.h"
#include "logger.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

FilesUI::FilesUI(MainWindow* mainWin)
    : QObject(mainWin)  // передаём родителя QObject
    , m_main(mainWin)
{}

// action buttons
void FilesUI::onAddFile()
{
    QListWidgetItem *algoItem = m_main->getUi()->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(m_main, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(m_main, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString destDirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QDir destDir(destDirPath);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QStringList files = QFileDialog::getOpenFileNames(
        m_main,
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
                    m_main,
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
                m_main,
                "Ошибка",
                QString("Не удалось скопировать файл:\n%1").arg(info.fileName())
                );
            continue;
        }
        // добавляем в список UI
        QListWidgetItem *item = new QListWidgetItem(info.fileName());
        item->setData(Qt::UserRole, destPath);
        m_main->getUi()->list_files->addItem(item);
        Logger::success(QString("Файл %1 успешно добавлен!").arg(info.fileName()));
    }
}
void FilesUI::onRemoveFile()
{
    QListWidgetItem *algoItem = m_main->getUi()->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(m_main, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(m_main, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString destDirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QDir destDir(destDirPath);
    if (!destDir.exists()) {
        destDir.mkpath(".");
    }

    QList<QListWidgetItem*> items = m_main->getUi()->list_files->selectedItems();

    if (items.isEmpty()) {
        QMessageBox::warning(m_main, "Ошибка", "Выберите файл(ы) для удаления");
        return;
    }

    // подтверждение
    QMessageBox::StandardButton reply =
        QMessageBox::question(
            m_main,
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
                    m_main,
                    "Ошибка",
                    QString("Не удалось удалить файл:\n%1")
                        .arg(QFileInfo(filePath).fileName())
                    );
                continue;
            }
        }
        // удаляем из списка UI
        delete m_main->getUi()->list_files->takeItem(m_main->getUi()->list_files->row(item));
        Logger::success(QString("Файл %1 успешно удален!").arg(QFileInfo(filePath).fileName()));
    }
}
void FilesUI::onClearAll()
{
    QListWidgetItem *algoItem = m_main->getUi()->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(m_main, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(m_main, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString dirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;

    QMessageBox::StandardButton reply =
        QMessageBox::question(
            m_main,
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
                m_main,
                "Ошибка",
                QString("Не удалось удалить файл:\n%1")
                    .arg(file)
                );
            continue;
        }
    }
    m_main->getUi()->list_files->clear();
    Logger::success("Все файлы алгоритма удалены");
}
void FilesUI::onOpenDir()
{
    QListWidgetItem *algoItem = m_main->getUi()->listAlgorithms->currentItem();
    if (!algoItem) {
        QMessageBox::warning(m_main, "Ошибка", "Сначала выберите алгоритм");
        return;
    }

    int algoNum = algoItem->data(Qt::UserRole).toInt();
    QString algoFolder = AlgorithmRegistry::get_folder(algoNum);
    if (algoFolder.isEmpty()) {
        QMessageBox::warning(m_main, "Ошибка", "Папка алгоритма не найдена");
        return;
    }

    QString dirPath = QString::fromStdString(dir_on_desktop.string()) + "/" + algoFolder;
    QDir dir(dirPath);
    if (!dir.exists()) {
        QMessageBox::warning(m_main, "Ошибка", "Папка алгоритма не существует");
        return;
    }
    // открыть в файловом менеджере
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
}

// контекстное меню
void FilesUI::showOutputContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_main->getUi()->list_files->itemAt(pos);
    QList<QListWidgetItem*> selected = m_main->getUi()->list_files->selectedItems();

    if (!item || selected.isEmpty())
        return;

    if (!item->isSelected()) {
        m_main->getUi()->listOutputFiles->clearSelection();
        item->setSelected(true);
        selected = m_main->getUi()->list_files->selectedItems();
    }

    QMenu menu(m_main);

    QAction *open = nullptr;
    QAction *openDir = nullptr;
    QAction *remove = nullptr;

    if (selected.size() == 1) {
        open = menu.addAction("Открыть файл");
        openDir = menu.addAction("Показать в папке");
        menu.addSeparator();
        remove = menu.addAction("Удалить");
    } else {
        open = menu.addAction("Открыть");
        menu.addSeparator();
        remove = menu.addAction("Удалить");
    }

    QAction *chosen = menu.exec(m_main->getUi()->listOutputFiles->mapToGlobal(pos));
    if (!chosen)
        return;

    // ===== ОТКРЫТЬ =====
    if (chosen == open) {
        for (auto *it : selected) {
            QString filePath = it->data(Qt::UserRole).toString();
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
        return;
    }

    // ===== ПОКАЗАТЬ В ПАПКЕ (ТОЛЬКО 1 ФАЙЛ) =====
    if (chosen == openDir && selected.size() == 1) {
        QString filePath = selected.first()->data(Qt::UserRole).toString();
        QFileInfo info(filePath);

#if defined(Q_OS_WIN)
        QStringList args;
        args << "/select," << QDir::toNativeSeparators(filePath);
        QProcess::startDetached("explorer.exe", args);

#elif defined(Q_OS_MAC)
        QProcess::startDetached("open", {"-R", filePath});

#else
        // Linux — честный fallback
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(info.absolutePath()));
#endif
        return;
    }

    // ===== УДАЛЕНИЕ =====
    if (chosen == remove) {

        QString text;
        if (selected.size() == 1) {
            text = QString("Удалить файл:\n%1 ?")
                       .arg(QFileInfo(
                                selected.first()->data(Qt::UserRole).toString()
                                ).fileName());
        } else {
            text = QString("Удалить выбранные файлы (%1 шт.)?")
                       .arg(selected.size());
        }

        if (QMessageBox::question(
                m_main,
                "Удаление файлов",
                text,
                QMessageBox::Yes | QMessageBox::No
                ) != QMessageBox::Yes)
            return;

        for (auto *it : selected) {
            QString path = it->data(Qt::UserRole).toString();
            QFile::remove(path);
            delete m_main->getUi()->listOutputFiles->takeItem(
                m_main->getUi()->listOutputFiles->row(it));
        }

        Logger::success(
            selected.size() == 1
                ? "Файл удалён"
                : "Файлы удалены");
    }
}
