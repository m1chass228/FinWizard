//list_algorithms.cpp
#include "list_algorithms.h"
#include "algorithmregistry.h"
#include "list_done_files.h"
#include "logger.h"
#include "utils.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

AlgoUI::AlgoUI(MainWindow* mainWin)
    : QObject(mainWin)  // передаём родителя QObject
    , m_main(mainWin)
{}

// algorithms into ui
void AlgoUI::loadAlgorithmsIntoUI()
{
    m_main->getUi()->listAlgorithms->clear();

    auto &map = AlgorithmRegistry::list();  // получаем ссылку на карту всех алгоритмов
    for (auto &[num, info] : map) {         // идем по всем зарегистрированным алгоритмам
        QListWidgetItem *item =             // создаем новый элемент для QListWidget
            new QListWidgetItem(QString("%1 - %2").arg(num).arg(info.name));    // num - номер алгоритма,
        // info - структура AlgorithmInfo в которой
        // name, folder, func
        item->setData(Qt::UserRole, num);   // сохраняем ключ алгоритма в item
        m_main->getUi()->listAlgorithms->addItem(item);  // выводим
    }
}

// Algorithms clicks
void AlgoUI::onAlgorithmSelected()
{
    m_main->getUi()->list_files->clear();

    QListWidgetItem *item = m_main->getUi()->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) return;

    int algoNum = item->data(Qt::UserRole).toInt();             // достаем номер алгоритма
    auto folder = AlgorithmRegistry::get_folder(algoNum);       // достаем папку алгоритма
    fs::path algo_dir = fs::path(get_dir_on_desktop()) / folder.toStdString();  // достаем папку с исходниками

    if (folder.isEmpty()) {
        m_main->getUi()->lblStatus->setText("Папка не найдена!");
        return;
    }
    for (const auto& entry : xlsx_in_path(algo_dir)) {
        QString fullPath = QString::fromStdString(entry.string());
        QString fileName = QString::fromStdString(entry);

        QListWidgetItem *fileItem = new QListWidgetItem(fileName);
        fileItem->setData(Qt::UserRole, fullPath);

        m_main->getUi()->list_files->addItem(fileItem);
    }
}

// Run button
void AlgoUI::onRunClicked()
{
    QListWidgetItem *item = m_main->getUi()->listAlgorithms->currentItem();  // получаем выбранный item в графическом списке
    if (!item) {
        m_main->getUi()->lblStatus->setText("Выберите алгоритм!");           // проверка выбора
        Logger::info("Выберите алгоритм!");
        return;
    }
    int algoNum = item->data(Qt::UserRole).toInt();             // достаем номер алгоритма

    //rm_rf_xlsx(dir_on_desktop);

    auto func = AlgorithmRegistry::get(algoNum);                // берем функцию по номеру
    if (!func) {
        m_main->getUi()->lblStatus->setText("Ошибка! функция не найдена!");
        return;
    }
    m_main->getUi()->lblStatus->setText("Выполнение......");
    Logger::info("Выполнение.....");

    // запуск
    func();
    // обновляем список готовых файлов
    m_main->getDoneUI()->loadDoneFilesIntoUI();

    m_main->getUi()->lblStatus->setText("Готово!");
    Logger::success("Готово!");
}
