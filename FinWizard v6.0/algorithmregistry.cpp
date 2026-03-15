#include "algorithmregistry.h"
#include <QDebug>
#include "utils.h"
#include "algorithmregistry.h"
#include <QString>
#include <filesystem>
namespace fs = std::filesystem;

std::map<int, AlgorithmInfo>& AlgorithmRegistry::getMap() {
    static std::map<int, AlgorithmInfo> map;
    return map;
}

void AlgorithmRegistry::registerAlgorithm(int num, const QString& name, const QString& folder, std::function<void()> func) {
    getMap()[num] = {name, folder, func};
    qDebug() << "Registered algorithm:" << num << name;
    createALLFolders();
}

std::function<void()> AlgorithmRegistry::get(int num) {
    auto& m = getMap();
    if (m.contains(num))
        return m[num].func;
    return nullptr;
}
QString AlgorithmRegistry::get_folder(int num) {
    auto& m = getMap();
    if (m.contains(num))
        return m[num].folder;
    return "";
}

std::string base_folder = "Зарплата";
fs::path dir_on_desktop = fs::path(get_desktop_path()) / base_folder ;

void AlgorithmRegistry::createALLFolders() {
    for (auto &[num, info] : list()) {
        fs::path dir = fs::path(get_desktop_path()) / "Зарплата" / info.folder.toStdString();
    try {
        fs:create_directories(dir);
            qDebug() << "created directories" << QString::fromStdString(dir.string());
    }
    catch (const fs::filesystem_error &e) {
            qWarning() << "Error creating folder for algo" << num << " (" << info.name << ") " << e.what();
    }
}
}
const std::map<int, AlgorithmInfo>& AlgorithmRegistry::list() {
    return getMap();
}
