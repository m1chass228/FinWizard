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
    // Начнём с папки, где лежит исполняемый файл
    QDir baseDir = QCoreApplication::applicationDirPath();

    // Максимум поднимемся на 5 уровней вверх (на всякий случай)
    for (int i = 0; i < 6; ++i) {
        QDir venvDir = baseDir;
        if (venvDir.cd("venv")) {
            // Проверяем наличие python в bin/ или Scripts/ (Windows)
            QString pythonPath;

#ifdef Q_OS_WIN
            pythonPath = venvDir.filePath("Scripts/python.exe");
#else
            pythonPath = venvDir.filePath("bin/python");
#endif

            if (QFile::exists(pythonPath)) {
                qDebug() << "Найден Python из venv:" << pythonPath;
                return pythonPath;
            }
        }

        // Если не нашли — поднимаемся на уровень выше
        if (!baseDir.cdUp()) {
            break; // уже в корне файловой системы
        }
    }

    // Если ничего не нашли
    qDebug() << "venv не найден!";
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
