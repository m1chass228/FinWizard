#include "finwizard/pluginmanager.h"
#include <QDebug>

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
    , m_settings("FinWizard", "PluginManager")
    , m_repository(m_settings) // Передаем настройки Складу
{
    // При старте программы автоматически сканируем кэш
    m_repository.refreshPlugins();
}

PluginManager::~PluginManager()
{
    // Нам не нужно писать код очистки памяти!
    // m_engine сам выгрузит все DLL в своем деструкторе.
    // m_settings сам сохранит данные на диск.
}

int PluginManager::addConfigFromZip(const QString &zipPath)
{
    // ВАЖНО: Если мы обновляем уже существующий плагин,
    // его нужно выгрузить из памяти ДО того, как Склад удалит его папку,
    // иначе на Windows возникнет ошибка блокировки файла (.dll).
    for (int id : m_repository.getAllConfigIds()) {
        const CachedConfig* cfg = m_repository.getConfig(id);
        if (cfg && cfg->originalZipPath == zipPath) {
            m_engine.unloadPlugin(id);
            break;
        }
    }

    // Делегируем работу Складу
    return m_repository.addConfigFromZip(zipPath);
}

QVariantMap PluginManager::runConfig(int id, const QVariantMap &params)
{
    // 1. Спрашиваем у Склада описание плагина
    const CachedConfig* cfg = m_repository.getConfig(id);
    if (!cfg || !cfg->isValid) {
        qWarning() << "Попытка запустить невалидный или несуществующий плагин ID:" << id;
        QVariantMap errorResult;
        errorResult["success"] = false;
        errorResult["error"] = QString("Попытка запустить невалидный или несуществующий плагин ID:").arg(id);
        return errorResult;
    }

    // 2. Отдаем конфиг Движку для выполнения
    return m_engine.runPlugin(*cfg, params);
}

void PluginManager::unloadPlugin(int id)
{
    m_engine.unloadPlugin(id);
}

QList<int> PluginManager::getAllConfigIds() const
{
    return m_repository.getAllConfigIds();
}

QStringList PluginManager::getDisplayNames() const
{
    QStringList names;

    for (int id : m_repository.getAllConfigIds()) {
        const CachedConfig* cfg = m_repository.getConfig(id);
        if (cfg) {
            // Формируем красивое имя для списков
            QString name = cfg->displayName;
            name += QString(" (ID %1)").arg(id);
            names << name;
        }
    }

    // Сортируем по алфавиту
    names.sort(Qt::CaseInsensitive);
    return names;
}

QMap<QString, QString> PluginManager::getConfigPreview(int id) const
{
    return m_repository.getConfigPreview(id);
}

QString PluginManager::getCacheBasePath() const
{
    return m_repository.getCacheBasePath();
}

void PluginManager::setCacheBasePath(const QString &path)
{
    // При смене папки нужно выгрузить все текущие плагины из памяти
    for (int id : m_repository.getAllConfigIds()) {
        m_engine.unloadPlugin(id);
    }

    m_repository.setCacheBasePath(path);
}

void PluginManager::refreshPlugins()
{
    m_repository.refreshPlugins();
}
