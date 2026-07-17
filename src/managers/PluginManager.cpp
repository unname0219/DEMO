#include "managers/PluginManager.h"
#include <QSettings>

PluginManager* PluginManager::s_instance = nullptr;

PluginManager* PluginManager::instance()
{
    if (!s_instance) {
        s_instance = new PluginManager();
    }
    return s_instance;
}

PluginManager::PluginManager()
{
    loadDefaultPlugins();
    loadInstalledFromSettings();
}

PluginManager::~PluginManager()
{
}

void PluginManager::loadDefaultPlugins()
{
    PluginInfo basicVideo;
    basicVideo.id = "basic-video";
    basicVideo.name = "基础视频格式包";
    basicVideo.description = "支持 MP4, MKV, AVI, MOV, WebM, FLV 等常见视频格式";
    basicVideo.version = "1.0.0";
    basicVideo.isInstalled = true;
    basicVideo.sizeMB = 2;
    basicVideo.supportedFormats = {"mp4", "mkv", "avi", "mov", "webm", "flv"};
    m_plugins[basicVideo.id] = basicVideo;

    PluginInfo basicAudio;
    basicAudio.id = "basic-audio";
    basicAudio.name = "基础音频格式包";
    basicAudio.description = "支持 MP3, WAV, FLAC, AAC, OGG, M4A 等常见音频格式";
    basicAudio.version = "1.0.0";
    basicAudio.isInstalled = true;
    basicAudio.sizeMB = 1;
    basicAudio.supportedFormats = {"mp3", "wav", "flac", "aac", "ogg", "m4a"};
    m_plugins[basicAudio.id] = basicAudio;

    PluginInfo basicImage;
    basicImage.id = "basic-image";
    basicImage.name = "基础图片格式包";
    basicImage.description = "支持 JPEG, PNG, GIF, BMP, WebP 等常见图片格式";
    basicImage.version = "1.0.0";
    basicImage.isInstalled = true;
    basicImage.sizeMB = 1;
    basicImage.supportedFormats = {"jpg", "jpeg", "png", "gif", "bmp", "webp"};
    m_plugins[basicImage.id] = basicImage;

    PluginInfo extendedVideo;
    extendedVideo.id = "extended-video";
    extendedVideo.name = "扩展视频格式包";
    extendedVideo.description = "支持 WMV, TS, M2TS, VOB, 3GP, RMVB 等扩展视频格式";
    extendedVideo.version = "1.0.0";
    extendedVideo.isInstalled = false;
    extendedVideo.sizeMB = 5;
    extendedVideo.supportedFormats = {"wmv", "ts", "m2ts", "vob", "3gp", "rmvb"};
    m_plugins[extendedVideo.id] = extendedVideo;

    PluginInfo extendedAudio;
    extendedAudio.id = "extended-audio";
    extendedAudio.name = "扩展音频格式包";
    extendedAudio.description = "支持 WMA, AIFF, Opus, ALAC, DSD 等扩展音频格式";
    extendedAudio.version = "1.0.0";
    extendedAudio.isInstalled = false;
    extendedAudio.sizeMB = 3;
    extendedAudio.supportedFormats = {"wma", "aiff", "opus", "alac", "dsd"};
    m_plugins[extendedAudio.id] = extendedAudio;

    PluginInfo extendedImage;
    extendedImage.id = "extended-image";
    extendedImage.name = "扩展图片格式包";
    extendedImage.description = "支持 TIFF, SVG, HEIC, AVIF, PSD 等扩展图片格式";
    extendedImage.version = "1.0.0";
    extendedImage.isInstalled = false;
    extendedImage.sizeMB = 4;
    extendedImage.supportedFormats = {"tiff", "svg", "heic", "avif", "psd"};
    m_plugins[extendedImage.id] = extendedImage;

    PluginInfo proVideo;
    proVideo.id = "pro-video";
    proVideo.name = "专业视频格式包";
    proVideo.description = "支持 ProRes, DNxHD, HEVC 10bit, AV1 等专业视频格式";
    proVideo.version = "1.0.0";
    proVideo.isInstalled = false;
    proVideo.sizeMB = 8;
    proVideo.supportedFormats = {"prores", "dnxhd", "hevc10", "av1"};
    m_plugins[proVideo.id] = proVideo;
}

void PluginManager::loadInstalledFromSettings()
{
    QSettings settings;
    QStringList installed = settings.value("plugins/installed", QStringList{
        "basic-video", "basic-audio", "basic-image"
    }).toStringList();

    for (auto& plugin : m_plugins) {
        plugin.isInstalled = installed.contains(plugin.id);
    }
}

QList<PluginInfo> PluginManager::availablePlugins() const
{
    return m_plugins.values();
}

QList<PluginInfo> PluginManager::installedPlugins() const
{
    QList<PluginInfo> result;
    for (const auto& plugin : m_plugins) {
        if (plugin.isInstalled) {
            result.append(plugin);
        }
    }
    return result;
}

bool PluginManager::isPluginInstalled(const QString& pluginId) const
{
    if (!m_plugins.contains(pluginId)) return false;
    return m_plugins[pluginId].isInstalled;
}

void PluginManager::installPlugin(const QString& pluginId)
{
    if (!m_plugins.contains(pluginId)) return;
    if (m_plugins[pluginId].isInstalled) return;

    emit installProgress(pluginId, 0);
    emit installProgress(pluginId, 50);

    m_plugins[pluginId].isInstalled = true;

    QSettings settings;
    QStringList installed;
    for (const auto& plugin : m_plugins) {
        if (plugin.isInstalled) {
            installed.append(plugin.id);
        }
    }
    settings.setValue("plugins/installed", installed);

    emit installProgress(pluginId, 100);
    emit pluginInstalled(pluginId);
}

void PluginManager::uninstallPlugin(const QString& pluginId)
{
    if (!m_plugins.contains(pluginId)) return;
    if (!m_plugins[pluginId].isInstalled) return;

    m_plugins[pluginId].isInstalled = false;

    QSettings settings;
    QStringList installed;
    for (const auto& plugin : m_plugins) {
        if (plugin.isInstalled) {
            installed.append(plugin.id);
        }
    }
    settings.setValue("plugins/installed", installed);

    emit pluginUninstalled(pluginId);
}
