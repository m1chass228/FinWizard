#include "finwizard/pluginengine.h"
#include <QCoreApplication>
#include <QDebug>

PluginEngine::~PluginEngine()
{
    m_activeConfigs.clear();
    for (auto &pair : m_loaders) {
        if (pair.second && pair.second->isLoaded()) {
            pair.second->unload();
        }
    }
    m_loaders.clear();
}

// ------------------ PREPARE DEPENCIES ----------------------

bool PluginEngine::prepareDependencies(const QString &cacheDir)
{
    // 1. Добавляем папку кэша в пути поиска библиотек Qt
    QCoreApplication::addLibraryPath(cacheDir);

    // 2. На Windows чаще всего этого недостаточно → модифицируем PATH
#ifdef Q_OS_WIN
    QByteArray oldPath = qgetenv("PATH");
    QByteArray pathToAdd = ";" + cacheDir.toUtf8();

    // Добавляем, только если этого пути там еще нет! (защита от утечки памяти ОС)
    if (!oldPath.contains(pathToAdd)) {
        qputenv("PATH", oldPath + pathToAdd);
        qDebug() << "PATH расширен для зависимостей:" << cacheDir;
    }
#endif

    // 3. На Linux — аналогично LD_LIBRARY_PATH
#ifdef Q_OS_LINUX
    QByteArray oldLd = qgetenv("LD_LIBRARY_PATH");
    QByteArray ldToAdd = ":" + cacheDir.toUtf8();

    if (!oldLd.contains(ldToAdd)) {
        qputenv("LD_LIBRARY_PATH", oldLd + ldToAdd);
        qDebug() << "LD_LIBRARY_PATH расширен:" << cacheDir;
    }
#endif

    return true;
}

// ------------------- LOAD PLUGIN ---------------------------

// ВАЖНО: Принимаем готовую структуру cfg, а не id!
bool PluginEngine::loadPlugin(const CachedConfig &cfg)
{
    // 1. Проверка конфига (Склад мог прислать битый)
    if (!cfg.isValid) return false;

    // 2. Если уже загружен — выходим (успех)
    // Используем count() так как это std::map
    if (m_loaders.count(cfg.id)) return true;

    // 3. Создаем лоадер (в cfg.entryPoint уже лежит полный абсолютный путь)
    auto loader = std::make_unique<QPluginLoader>(cfg.entryPoint);

    // Подготовка окружения (пути к либам)
    prepareDependencies(cfg.cachePath);

    if (!loader->load()) {
        qWarning() << "Ошибка загрузки плагина:" << loader->errorString();
        return false;
    }

    // 4. Получаем объект
    QObject *obj = loader->instance();
    if (!obj) {
        loader->unload(); // Обязательно выгружаем!
        qWarning() << "Ошибка: loader не смог создать экземпляр объекта!";
        return false;
    }

    IConfig *config = qobject_cast<IConfig*>(obj);
    if (!config) {
        loader->unload(); // Обязательно выгружаем!
        qWarning() << "Критическая ошибка: Плагин загружен, но интерфейс IConfig не распознан. Проверьте IID!";
        return false;
    }

    // 5. Сохраняем и лоадер, и указатель на интерфейс
    m_activeConfigs[cfg.id] = config;
    m_loaders[cfg.id] = std::move(loader);

    qInfo() << "Плагин успешно загружен:" << cfg.displayName;
    return true;
}

// ------------------ UNLOAD PLUGIN ---------------------------

void PluginEngine::unloadPlugin(int id)
{
    // Сначала удаляем интерфейс из списка активных
    m_activeConfigs.erase(id);

    auto it = m_loaders.find(id);
    if (it == m_loaders.end()) return;

    if (it->second && it->second->isLoaded()) {
        it->second->unload();
        qDebug() << "Плагин" << id << "выгружен";
    }

    // Удаляем лоадер
    m_loaders.erase(it);
}

// ---------------- RUN PLUGIN ---------------------------------

QVariantMap PluginEngine::runPlugin(const CachedConfig &cfg, const QVariantMap &params)
{
    // Движок сам подгрузит плагин, если он еще не в памяти
    if (!loadPlugin(cfg)) {
        QVariantMap errorResult;
        errorResult["success"] = false;
        errorResult["error"] = QString("Не удалось загрузить плагин '%1' в память").arg(cfg.displayName);
        return errorResult;
    }

    IConfig *plugin = m_activeConfigs[cfg.id];
    QVariantMap result = plugin->execute(params);

    // Просто логируем для консоли разработчика, а сам результат прокидываем наверх в GUI
    if (result.value("success").toBool()) {
        qInfo() << "Успех:" << result.value("message").toString();
    } else {
        qWarning() << "Ошибка плагина:" << result.value("error").toString();
    }

    return result;
}
