#ifndef PLUGINENGINE_H
#define PLUGINENGINE_H

#include <QVariantMap>
#include <QPluginLoader>
#include <map>
#include <memory>
#include "finwizard/iconfig.h"
#include "finwizard/pluginrepository.h" // Нужен только ради структуры CachedConfig

class PluginEngine
{
public:
    PluginEngine() = default;
    ~PluginEngine();

    // Загружает плагин в память
    bool loadPlugin(const CachedConfig &cfg);

    // Выгружает плагин из памяти
    void unloadPlugin(int id);

    // Запускает метод execute()
    QVariantMap runPlugin(const CachedConfig &cfg, const QVariantMap &params);

private:
    bool prepareDependencies(const QString &cacheDir);

    std::map<int, std::unique_ptr<QPluginLoader>> m_loaders;
    std::map<int, IConfig*> m_activeConfigs;
};

#endif // PLUGINENGINE_H
