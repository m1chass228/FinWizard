//list_done_files.cpp
#include "list_done_files.h"
#include "logger.h"
#include "utils.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <qmessagebox.h>
#include <QDesktopServices>

DoneUI::DoneUI(MainWindow* mainWin)
    : QObject(mainWin)  // передаём родителя QObject
    , m_main(mainWin)
    , m_watcher(new QFileSystemWatcher(this))  // Создаём watcher с родителем this
{
    // Добавляем путь к папке для мониторинга
    if (!m_watcher->addPath(QString::fromStdString(get_dir_on_desktop()))) {
        Logger::error(QString("Не удалось мониторить папку: %1").arg(QString::fromStdString(get_dir_on_desktop())));
    }

    // Подключаем сигнал directoryChanged к слоту loadDoneFilesIntoUI
    //connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &DoneUI::loadDoneFilesIntoUI);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &path) {
        Logger::info("directoryChanged emitted для пути: " + path);
        loadDoneFilesIntoUI();
    });
    loadDoneFilesIntoUI();
}

// done files into ui
void DoneUI::loadDoneFilesIntoUI()
{
    Logger::info("Watcher сработал: directoryChanged. Текущих items: " + QString::number(m_main->getUi()->listOutputFiles->count()));
    m_main->getUi()->listOutputFiles->clear();

    for (const auto& entry : xlsx_in_path(get_dir_on_desktop())) {
        QString fullPath = QString::fromStdString(entry.string());
        QString fileName = QString::fromStdString(entry.filename().string());

        QListWidgetItem *fileItem = new QListWidgetItem(fileName);
        fileItem->setData(Qt::UserRole, fullPath);

        m_main->getUi()->listOutputFiles->addItem(fileItem);
    }
}
// context menu for done files
void DoneUI::showOutputContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_main->getUi()->listOutputFiles->itemAt(pos);
    QList<QListWidgetItem*> selected = m_main->getUi()->listOutputFiles->selectedItems();

    if (!item || selected.isEmpty())
        return;

    // Если кликнули ПКМ по НЕ выделенному — выделяем только его
    if (!item->isSelected()) {
        m_main->getUi()->listOutputFiles->clearSelection();
        item->setSelected(true);
        selected = m_main->getUi()->listOutputFiles->selectedItems();
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

        for (int i = selected.size() - 1; i >= 0; --i) {
            auto *it = selected.at(i);
            QString path = it->data(Qt::UserRole).toString();
            if (QFile::remove(path)) {
                delete m_main->getUi()->listOutputFiles->takeItem(
                    m_main->getUi()->listOutputFiles->row(it));
            } else {
                Logger::error("Не удалось удалить файл: " + path);
            }
        }
        loadDoneFilesIntoUI();
        Logger::success(
            selected.size() == 1
                ? "Файл удалён"
                : "Файлы удалены");
    }

}
