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

    m_videoFormats = settings.value("video", QStringList{
        "mp4", "mkv", "avi", "mov", "webm", "flv"
    }).toStringList();

    m_audioFormats = settings.value("audio", QStringList{
        "mp3", "wav", "flac", "aac", "ogg", "m4a"
    }).toStringList();

    m_imageFormats = settings.value("image", QStringList{
        "jpg", "jpeg", "png", "gif", "bmp", "webp"
    }).toStringList();

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
