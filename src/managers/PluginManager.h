#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QStringList>
#include <QMap>

struct PluginInfo {
    QString id;
    QString name;
    QString description;
    QString version;
    bool isInstalled;
    int sizeMB;
    QStringList supportedFormats;
};

class PluginManager : public QObject
{
    Q_OBJECT

public:
    static PluginManager* instance();

    QList<PluginInfo> availablePlugins() const;
    QList<PluginInfo> installedPlugins() const;

    bool isPluginInstalled(const QString& pluginId) const;
    void installPlugin(const QString& pluginId);
    void uninstallPlugin(const QString& pluginId);

signals:
    void pluginInstalled(const QString& pluginId);
    void pluginUninstalled(const QString& pluginId);
    void installProgress(const QString& pluginId, int percent);

private:
    PluginManager();
    ~PluginManager();

    static PluginManager* s_instance;
    QMap<QString, PluginInfo> m_plugins;

    void loadDefaultPlugins();
    void loadInstalledFromSettings();
};

#endif
