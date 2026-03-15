#include "algorithmregistry.h"
#include "utils.h"
#include "logger.h"

namespace {
    int algo_number = 1;
    QString name = "Марьино Коломенская сводный отчёт";
    std::string dir = "Марьино_Коломенская";
    std::string filename = "Марьино_Коломенская_сводный_отчет.xlsx";
    QString pythonScript = getAlgorithmScriptPath("marjino_kolomenskaya_svodni/marjino_kolomenskaya_svodni.py");
}

void algo1() {
    Logger::info(QString("Запуск алгоритма: %1").arg(name));
    try {
        // 1. Находим Python из venv
        QString python_path = findProjectVenvPython();
        if (python_path.isEmpty()) {
            throw std::runtime_error("Не удалось найти виртуальное окружение (venv) в корне проекта.");
        }
        Logger::info("Используется Python: " + python_path);

        // 2. Находим скрипт
        QFileInfo scriptInfo(pythonScript);
        if (!scriptInfo.exists() || !scriptInfo.isFile()) {
            throw std::runtime_error("Python-скрипт не найден:\n" + pythonScript.toStdString());
        }
        Logger::info("Скрипт найден: " + pythonScript);

        // 3. Папки с данными
        QString output_xlsx = QString::fromStdString((dir_on_desktop / filename).string()); // название конечного
        fs::path dir_path = fs::path(get_desktop_path()) / base_folder / dir;

        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            throw std::runtime_error("Папка с данными не найдена на рабочем столе:\n" + dir_path.string());
        }
        Logger::info("Папка с данными: " + QString::fromStdString(dir_path.string()));

        // 4. Собираем список .xlsx файлов
        QStringList file_names;                    // достаем пути к xlsx файлам и передаем в python
        int file_count = 0;
        for (const auto& entry : xlsx_in_path(dir_path)) {
            file_names << "\"" + QString::fromStdString(entry.string()) + "\"";
            file_count++;
        }

        if (file_count == 0) {
            throw std::runtime_error("В папке не найдено ни одного .xlsx файла.");
        }

        QString pyList = "[" + file_names.join(", ") + "]";

        Logger::info("Запуск Python-скрипта...");
        Logger::info(QString("[FinWizard] Запуск алгоритма: %1").arg(name));
        Logger::info(QString("Python: %1").arg(python_path));
        Logger::info(QString("Скрипт: %1").arg(pythonScript));
        Logger::info(QString("Файлов найдено: %1").arg(file_count));
        Logger::info(QString("Выходной файл: %1").arg(output_xlsx));

        // 5. Запускаем Python
        QProcess process;
        process.setProgram(python_path);
        process.setArguments(QStringList() << "-u" << pythonScript << pyList << output_xlsx);
        process.setWorkingDirectory(scriptInfo.absolutePath());

        // 6. Ловим выводы
        QObject::connect(&process, &QProcess::readyReadStandardOutput, [&]() {
            QString out = process.readAllStandardOutput();
            for (const QString& line : out.split('\n', Qt::SkipEmptyParts)) {
                Logger::log(line, "gray");
            }
        });

        QObject::connect(&process, &QProcess::readyReadStandardError, [&]() {
            QString err = process.readAllStandardError();
            for (const QString& line : err.split('\n', Qt::SkipEmptyParts)) {
                Logger::error(line);
            }
        });

        process.start();
        if (!process.waitForStarted(10000)) { // 10 секунд на запуск
            throw std::runtime_error("Не удалось запустить Python-процесс.\nПроверьте, активировано ли venv и установлен ли Python.");
        }

        // Ждём завершения (с таймаутом на случай зависания)
        if (!process.waitForFinished(300000)) { // 5 минут максимум
            process.kill();
            throw std::runtime_error("Python-скрипт завис или выполнялся слишком долго (>5 мин). Процесс принудительно завершён.");
        }

        if (process.exitCode() != 0) {
            throw std::runtime_error("Python-скрипт завершился с ошибкой (код: " + std::to_string(process.exitCode()) + ")");
        }

        // ВСЁ ОК!
        Logger::success(QString("ГОТОВО! %1 успешно создан!").arg(name));
        Logger::info(
            QString("Файл сохранён на рабочий стол: %1").arg(output_xlsx)
            );
    }
    catch (const std::exception& e) {
        QString errorText = QString::fromStdString(e.what());
        Logger::error("ОШИБКА: " + errorText);
    }
    catch (...) {
        Logger::error("КРИТИЧЕСКАЯ ОШИБКА: Неизвестная ошибка при выполнении алгоритма.");
    }
}
REGISTER_ALGORITHM(algo_number, name, QString::fromStdString(dir), algo1);

