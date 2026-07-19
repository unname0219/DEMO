#include "managers/FileAssociation.h"
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>

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
    QString exePath = QCoreApplication::applicationFilePath();
    QString progId = "FireflyPlayer";

    QSettings progIdSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(progId),
                            QSettings::NativeFormat);
    progIdSettings.setValue(".Default", "Firefly Player");
    progIdSettings.setValue("FriendlyTypeName", "Firefly Player");

    progIdSettings.beginGroup("shell");
    progIdSettings.beginGroup("open");
    progIdSettings.beginGroup("command");
    progIdSettings.setValue(".Default", QString("\"%1\" \"%2\"").arg(exePath, "%1"));
    progIdSettings.endGroup();
    progIdSettings.endGroup();
    progIdSettings.endGroup();

    QStringList allFormats = m_videoFormats + m_audioFormats + m_imageFormats;
    foreach (const QString& ext, allFormats) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        extSettings.setValue(".Default", progId);

        QSettings openWithSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1\\OpenWithProgIDs")
                                   .arg(extKey), QSettings::NativeFormat);
        openWithSettings.setValue(progId, "");

        QSettings choiceSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1\\OpenWithList")
                                 .arg(extKey), QSettings::NativeFormat);
        choiceSettings.setValue(progId, "");
    }

    QSettings appUserModelSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1\\Application")
                                   .arg(progId), QSettings::NativeFormat);
    appUserModelSettings.setValue("AppUserModelId", "FireflyPlayer");

    QSettings iconSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1\\DefaultIcon")
                           .arg(progId), QSettings::NativeFormat);
    iconSettings.setValue(".Default", QString("\"%1\",0").arg(exePath));
}

void FileAssociation::unregisterAssociations()
{
    QString progId = "FireflyPlayer";

    QStringList allFormats = m_videoFormats + m_audioFormats + m_imageFormats;
    foreach (const QString& ext, allFormats) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        QString currentProgId = extSettings.value(".Default").toString();
        if (currentProgId == progId) {
            extSettings.remove(".Default");
        }

        QSettings openWithSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1\\OpenWithProgIDs")
                                   .arg(extKey), QSettings::NativeFormat);
        openWithSettings.remove(progId);
    }
}

bool FileAssociation::checkAssociationMatches() const
{
    QString progId = "FireflyPlayer";
    QStringList allFormats = m_videoFormats + m_audioFormats + m_imageFormats;

    foreach (const QString& ext, allFormats) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        QString currentProgId = extSettings.value(".Default").toString();
        if (currentProgId != progId) {
            return false;
        }
    }

    return true;
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

void FileAssociation::syncFromSystem()
{
    QString progId = "FireflyPlayer";

    static const QStringList knownVideo = {"mp4", "mkv", "avi", "mov", "webm", "flv", "wmv", "m4v", "ts", "3gp"};
    static const QStringList knownAudio = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "ape", "opus"};
    static const QStringList knownImage = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "tiff", "ico"};

    QStringList newVideo;
    QStringList newAudio;
    QStringList newImage;

    foreach (const QString& ext, knownVideo) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        QString currentProgId = extSettings.value(".Default").toString();
        if (currentProgId == progId) {
            newVideo.append(ext);
        }
    }

    foreach (const QString& ext, knownAudio) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        QString currentProgId = extSettings.value(".Default").toString();
        if (currentProgId == progId) {
            newAudio.append(ext);
        }
    }

    foreach (const QString& ext, knownImage) {
        QString extKey = QString(".%1").arg(ext);
        QSettings extSettings(QString("HKEY_CURRENT_USER\\Software\\Classes\\%1").arg(extKey),
                              QSettings::NativeFormat);
        QString currentProgId = extSettings.value(".Default").toString();
        if (currentProgId == progId) {
            newImage.append(ext);
        }
    }

    m_videoFormats = newVideo;
    m_audioFormats = newAudio;
    m_imageFormats = newImage;
    saveToSettings();
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
