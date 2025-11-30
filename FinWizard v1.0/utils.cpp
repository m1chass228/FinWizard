#include "utils.h"
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
