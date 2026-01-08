#ifndef ALGORITHMREGISTRY_H
#define ALGORITHMREGISTRY_H

#endif // ALGORITHMREGISTRY_H
#pragma once
#include <QObject>
#include <map>
#include <functional>
#include <QString>
#include <filesystem>
#include <QDebug>
#include <xlsxdocument.h>

namespace fs = std::filesystem;

struct AlgorithmInfo {
    QString name;
    QString folder;
    std::function<void()> func;
};

class AlgorithmRegistry {
public:
    static void registerAlgorithm(int num, const QString& name, const QString& folder, std::function<void()>func);
    static std::function<void()> get(int num);
    static QString get_folder(int num);
    static const std::map<int, AlgorithmInfo>& list();
    static void createALLFolders();
private:
    void createALLfolders();
    static std::map<int, AlgorithmInfo>& getMap();
};

#define REGISTER_ALGORITHM(num, name, folder, func) \
static bool _reg_##func = [](){ \
        AlgorithmRegistry::registerAlgorithm(num, name, folder, func); \
        return true; \
}()
