#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QSettings>

#include "finwizard/pluginrepository.h"
#include "finwizard/pluginengine.h"

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager() override;

    // --- Основное API ---
    int addConfigFromZip(const QString &zipPath);
    QVariantMap runConfig(int id, const QVariantMap &params);
    void unloadPlugin(int id);

    // --- Информация для UI ---
    QList<int> getAllConfigIds() const;
    QStringList getDisplayNames() const;
    QMap<QString, QString> getConfigPreview(int id) const;

    // --- Управление кэшем ---
    QString getCacheBasePath() const;
    void setCacheBasePath(const QString &path);
    void refreshPlugins();

private:
    QSettings m_settings;
    PluginRepository m_repository;
    PluginEngine m_engine;
};

#endif // PLUGINMANAGER_H
