#include "finwizard/pluginrepository.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

PluginRepository::PluginRepository(QSettings &settings) : m_settings(settings) {}

// ----------------- AVAILABLE ID ------------------------------

int PluginRepository::nextAvailableId() const {
    return m_configs.empty() ? 1 : m_configs.rbegin()->first + 1;
}

// ----------------- GET CONFIGS --------------------------------

const CachedConfig* PluginRepository::getConfig(int id) const {
    auto it = m_configs.find(id);
    return (it != m_configs.end()) ? &it->second : nullptr;
}

QList<int> PluginRepository::getAllConfigIds() const {
    QList<int> ids;
    for (const auto &[id, cfg] : m_configs) ids << id;
    return ids;
}



// ----------------- ADD CONFIG (ZIP)----------------------------

int PluginRepository::addConfigFromZip(const QString &zipath)
{
    if (!QFile::exists(zipath)) return -1;

    QFileInfo fi(zipath);
    QString zipBase = fi.baseName();

    // Проверяем, есть ли уже такой zip
    int existingId = -1;
    for (const auto &[id, cfg] : m_configs) {
        if (cfg.originalZipPath == zipath) {
            existingId = id;
            break;
        }
    }

    // Если есть и zip не новее — возвращаем старый
    if (existingId != -1) {
        const auto &old = m_configs[existingId];
        if (fi.lastModified() <= old.lastExtracted) {
            m_configs[existingId].lastUsed = QDateTime::currentDateTime();
            return existingId;
        }
        // Иначе удаляем старое
        QDir(old.cachePath).removeRecursively();
        m_configs.erase(existingId);
    }

    int id = nextAvailableId();

    QString cacheDir = createCacheDirForId(id);
    if (cacheDir.isEmpty()) return -1;

    if (!extractZip(zipath, cacheDir)) {
        QDir(cacheDir).removeRecursively();
        return -1;
    }

    CachedConfig cfg = parseManifest(cacheDir);
    if (!cfg.isValid) {
        QDir(cacheDir).removeRecursively();
        return -1;
    }

    cfg.id = id;
    cfg.lastExtracted = QDateTime::currentDateTime();
    cfg.lastUsed = cfg.lastExtracted;

    m_configs[id] = std::move(cfg);

    return id;
}

bool PluginRepository::extractZip(const QString &zipPath, const QString &targetDir)
{
    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning() << "ZIP open error:" << zip.getZipError();
        return false;
    }

    QDir dir(targetDir);
    if (!dir.mkpath(".")) return false;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        QString name = zip.getCurrentFileName();
        if (name.endsWith('/')) continue;

        QString outPath = targetDir + QDir::separator() + name;
        QDir().mkpath(QFileInfo(outPath).absolutePath());

        QuaZipFile zf(&zip);
        if (!zf.open(QIODevice::ReadOnly)) continue;

        QFile out(outPath);
        if (!out.open(QIODevice::WriteOnly)) continue;

        out.write(zf.readAll());
        out.close();
    }
    zip.close();
    return true;
}

// ----------------- REMOVE CONFIG  -------------------------------

void PluginRepository::removeConfig(int id)
{
    auto it = m_configs.find(id);
    if (it != m_configs.end()) {
        QDir(it->second.cachePath).removeRecursively();
        m_configs.erase(it);
    }
}

// -----------------  REFRESH PLUGINS  ----------------------------
void PluginRepository::refreshPlugins()
{
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (defaultPath.isEmpty()) {
        defaultPath = QDir::tempPath() + "/FinWizard_cache";
    }
    defaultPath = QDir::cleanPath(defaultPath + "/plugins-cache");

    // Читаем из настроек (если ничего нет → вернёт дефолт)
    QString m_cacheBasePath = m_settings.value("cache/path", defaultPath).toString();

    qDebug() << "--- SCANNING CACHE ---";
    qDebug() << "Path:" << m_cacheBasePath;

    qDebug() << "Обновление списка плагинов из:" << m_cacheBasePath;

    // 1. Очищаем текущий кэш описаний (НЕ лоадеры!)
    // Мы оставляем m_loaders и m_activeConfigs нетронутыми,
    // чтобы работающие плагины не "отвалились".
    m_configs.clear();

    QDir cacheDir(m_cacheBasePath);
    if (!cacheDir.exists()) {
        qDebug() << "CRITICAL: Cache directory does not exist!";
        return;
    }
    // 2. Итерируемся по подпапкам (каждая подпапка - один плагин)
    QStringList subDirs = cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug() << "Found subdirectories:" << subDirs;

    for (const QString &dirName : subDirs) {
        int id = dirName.toInt(); // если у тебя папки называются по ID
        if (id <= 0) continue;

        QString fullPath = cacheDir.absoluteFilePath(dirName);

        // Вызываем твою логику чтения манифеста
        CachedConfig cfg = parseManifest(fullPath);
        if (cfg.isValid) {
            cfg.id = id;
            m_configs[id] = cfg;
        }
    }

    qDebug() << "Найдено валидных конфигов:" << m_configs.size();
}

// ------------------ CACHE ---------------------

QString PluginRepository::getCacheBasePath() const
{
    // дефолт
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (defaultPath.isEmpty()) {
        defaultPath = QDir::tempPath() + "/FinWizard_cache";
    }
    defaultPath = QDir::cleanPath(defaultPath + "/plugins-cache");

    // Читаем из настроек (если ничего нет → вернёт дефолт)
    QString savedPath = m_settings.value("cache/path", defaultPath).toString();

    // Нормализуем и проверяем
    savedPath = QDir::cleanPath(savedPath);

    // Если путь пустой или недоступный — возвращаем дефолт и сбрасываем настройку
    if (savedPath.isEmpty() || !QDir(savedPath).mkpath(".")) {
        const_cast<QSettings&>(m_settings).remove("cache/path");  // const_cast для mutable  // чистим битую настройку
        return defaultPath;
    }

    return savedPath;

}

void PluginRepository::setCacheBasePath(const QString &path)
{
    if (path.isEmpty()) {
        m_settings.remove("cache/path");
    } else {
        QString cleaned = QDir::cleanPath(path);
        if (QDir(cleaned).mkpath(".")) {
            m_settings.setValue("cache/path", cleaned);
            m_settings.sync();
        } else {
            qWarning() << "Не удалось применить путь кэша:" << cleaned;
        }
    }
    refreshPlugins();
}

QString PluginRepository::createCacheDirForId(int id)
{
    QString base = getCacheBasePath();
    QString dirPath = base + QDir::separator() + QString::number(id);

    QDir dir(dirPath);
    if (!dir.mkpath(".")) {
        qWarning() << "Не удалось создать папку кэша:" << dirPath;
            return {};
    }
    return dirPath;
}

// ------------------ PARSE MANIFEST ----------------

CachedConfig PluginRepository::parseManifest(const QString &dirPath) const
{
    CachedConfig cfg;
    cfg.isValid = false;
    cfg.cachePath = dirPath;

    QFile file(dirPath + "/manifest.json");
    if (!file.open(QIODevice::ReadOnly)) {
        cfg.validationMessage = "Не удалось открыть manifest.json";
        return cfg;
    }

    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    cfg.displayName = obj.value("name").toString().trimmed();
    cfg.description = obj.value("description").toString().trimmed();
    cfg.configType = obj.value("type").toString("cpp-plugin").trimmed();

    // --- УМНЫЙ ПОИСК БИНАРНИКА (КРОССПЛАТФОРМА) ---
    QString dllName;

#ifdef Q_OS_WIN
    if (obj.contains("entry_win")) {
        dllName = obj.value("entry_win").toString().trimmed();
    }
#elif defined(Q_OS_LINUX)
    if (obj.contains("entry_linux")) {
        dllName = obj.value("entry_linux").toString().trimmed();
    }
#elif defined(Q_OS_MAC)
    if (obj.contains("entry_mac")) {
        dllName = obj.value("entry_mac").toString().trimmed();
    }
#endif

    // Fallback: Если ОС-специфичного ключа нет или он пустой, берем универсальный "entry"
    if (dllName.isEmpty() && obj.contains("entry")) {
        dllName = obj.value("entry").toString().trimmed();
    }
    // ----------------------------------------------

    if (!dllName.isEmpty()) {
        cfg.entryPoint = QDir(dirPath).absoluteFilePath(dllName);

        if (QFile::exists(cfg.entryPoint)) {
            cfg.isValid = true;
            cfg.validationMessage = "OK";
        } else {
            cfg.validationMessage = "Файл плагина не найден: " + dllName;
        }
    } else {
        cfg.validationMessage = "В манифесте не указан ключ 'entry' или специфичный для ОС";
    }

    return cfg;
}

// ----------------- GET PREVIEW -----------------------

QMap<QString, QString> PluginRepository::getConfigPreview(int id) const
{
    QMap<QString, QString> preview; // Исправил опечатку в prewiew

    // 1. Ищем конфиг в памяти (быстро)
    auto it = m_configs.find(id);
    if (it == m_configs.end() || !it->second.isValid) {
        return preview; // Возвращаем пустую карту
    }

    const CachedConfig &cfg = it->second;

    // 2. Заполняем данными из уже загруженного конфига (НЕ читаем диск лишний раз!)
    preview["name"] = cfg.displayName;
    preview["description"] = cfg.description;
    preview["type"] = cfg.configType;

    // 3. А вот иконку ищем на диске (так как путь к ней мы не хранили)
    QStringList iconNames = {
        "preview.png", "icon.png", "thumbnail.png", "cover.png",
        "preview.jpg", "icon.jpg", "preview.jpeg"
    };

    QDir dir(cfg.cachePath);
    for (const QString &name : iconNames) {
        QString fullPath = dir.absoluteFilePath(name);
        if (QFile::exists(fullPath)) {
            preview["icon"] = fullPath;
            break; // Нашли первую попавшуюся — хватит
        }
    }

    return preview;
}




