#include "utils.h"
#include <iostream>
#include <qdir.h>
std::string get_desktop_path() {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, path))) {
        return std::string(path);
    } else {
        return std::string();
    }
#elif __APPLE__
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/Desktop";
    } else {
        return std::string();
    }
#else // Linux/Unix
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home) + "/Desktop";
    } else {
        return std::string();
    }
#endif
}

std::vector<fs::path> xlsx_in_path(const fs::path& folder) {
    std::vector<fs::path> names;

    if (!fs::exists(folder)) {
        qWarning() << "Папка не найдена!" << QString::fromStdString(folder.string());
        return names;
    }
    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;
        fs::path file_path = entry.path();
        if (file_path.extension() == ".xlsx" || file_path.extension() == ".XLSX") {
            names.push_back(file_path);
        }
    }
    return names;
}

void rm_rf_xlsx(const fs::path& folder) {
    try {
        for (const auto& entry : fs::directory_iterator(folder)){
            if (fs::is_regular_file(entry.status())) {
                fs::path file_path = entry.path();
                if (file_path.extension() == ".xlsx" || file_path.extension() == ".XLSX")
                    fs::remove(entry.path());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cout << e.what();
    }
}

QString findProjectVenvPython()
{
    // 1. Берём директорию исполняемого файла
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    qDebug() << "applicationDirPath:" << appDir;

    // 2. Поднимаемся вверх до уровня, где лежит "FinWizard v5.0" (или любой маркер проекта)
    int levels = 0;
    const int MAX_UP = 8;
    bool foundProjectRoot = false;

    while (levels < MAX_UP && dir.cdUp()) {
        levels++;
        if (dir.dirName() == "FinWizard v5.0") {
            foundProjectRoot = true;
            qDebug() << "Нашли корень проекта на уровне" << levels << ":" << dir.absolutePath();
            break;
        }
    }

    if (!foundProjectRoot) {
        qDebug() << "Не удалось найти корень проекта (FinWizard v5.0)";
        return "";
    }

    // 3. Теперь ищем venv прямо в корне проекта
    QStringList venvNames = {"venv", ".venv", "env"};

    for (const QString &name : venvNames) {
        QString venvFull = dir.filePath(name);
        QDir venvDir(venvFull);

        if (venvDir.exists()) {
            qDebug() << "Найдена папка" << name << ":" << venvFull;

            QString pythonPath;
#ifdef Q_OS_WIN
            pythonPath = venvDir.filePath("Scripts/python.exe");
#else
            pythonPath = venvDir.filePath("bin/python3");  // чаще python3 в новых venv
            if (!QFile::exists(pythonPath))
                pythonPath = venvDir.filePath("bin/python");
#endif

            if (QFile::exists(pythonPath)) {
                qDebug() << "Найден исполняемый Python:" << pythonPath;
                return pythonPath;
            } else {
                qDebug() << "В" << venvFull << "нет python / python3";
            }
        }
    }

    qDebug() << "venv не найдена в корне проекта:" << dir.absolutePath();
    return "";
}

QString getAlgorithmScriptPath(const QString& relativePath)
{
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp(); // build
    dir.cdUp(); // FinWizard v?.0
    dir.cd("algorithms");

    return dir.absoluteFilePath(relativePath);
}
