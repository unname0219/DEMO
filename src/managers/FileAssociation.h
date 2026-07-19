#ifndef FILEASSOCIATION_H
#define FILEASSOCIATION_H

#include <QObject>
#include <QStringList>

class FileAssociation : public QObject
{
    Q_OBJECT

public:
    static FileAssociation* instance();

    QStringList videoFormats() const;
    QStringList audioFormats() const;
    QStringList imageFormats() const;

    void setVideoFormats(const QStringList& formats);
    void setAudioFormats(const QStringList& formats);
    void setImageFormats(const QStringList& formats);

    void clearAssociations();

    void registerAssociations();
    void unregisterAssociations();
    bool checkAssociationMatches() const;

    bool isAssociated(const QString& extension) const;

    void syncFromSystem();

private:
    FileAssociation();
    ~FileAssociation();

    static FileAssociation* s_instance;

    QStringList m_videoFormats;
    QStringList m_audioFormats;
    QStringList m_imageFormats;

    void loadFromSettings();
    void saveToSettings();
};

#endif
