#include "managers/FileAssociation.h"
#include <QSettings>

FileAssociation* FileAssociation::s_instance = nullptr;

FileAssociation* FileAssociation::instance()
{
    if (!s_instance) {
        s_instance = new FileAssociation();
    }
    return s_instance;
}

FileAssociation::FileAssociation()
{
    loadFromSettings();
}

FileAssociation::~FileAssociation()
{
}

QStringList FileAssociation::videoFormats() const
{
    return m_videoFormats;
}

QStringList FileAssociation::audioFormats() const
{
    return m_audioFormats;
}

QStringList FileAssociation::imageFormats() const
{
    return m_imageFormats;
}

void FileAssociation::setVideoFormats(const QStringList& formats)
{
    m_videoFormats = formats;
    saveToSettings();
}

void FileAssociation::setAudioFormats(const QStringList& formats)
{
    m_audioFormats = formats;
    saveToSettings();
}

void FileAssociation::setImageFormats(const QStringList& formats)
{
    m_imageFormats = formats;
    saveToSettings();
}

void FileAssociation::clearAssociations()
{
    m_videoFormats.clear();
    m_audioFormats.clear();
    m_imageFormats.clear();
    saveToSettings();
}

void FileAssociation::registerAssociations()
{
}

void FileAssociation::unregisterAssociations()
{
}

bool FileAssociation::isAssociated(const QString& extension) const
{
    QString ext = extension.toLower();
    if (ext.startsWith(".")) {
        ext = ext.mid(1);
    }
    return m_videoFormats.contains(ext)
        || m_audioFormats.contains(ext)
        || m_imageFormats.contains(ext);
}

void FileAssociation::loadFromSettings()
{
    QSettings settings;
    settings.beginGroup("fileAssociations");

    // 默认为空：只有用户明确选择（首次启动点"开始使用"）才会有关联，
    // 跳过时保持为空，避免"自欺欺人"地显示未真实关联的格式。
    m_videoFormats = settings.value("video", QStringList{}).toStringList();
    m_audioFormats = settings.value("audio", QStringList{}).toStringList();
    m_imageFormats = settings.value("image", QStringList{}).toStringList();

    settings.endGroup();
}

void FileAssociation::saveToSettings()
{
    QSettings settings;
    settings.beginGroup("fileAssociations");
    settings.setValue("video", m_videoFormats);
    settings.setValue("audio", m_audioFormats);
    settings.setValue("image", m_imageFormats);
    settings.endGroup();
}
