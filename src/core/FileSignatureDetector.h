#ifndef FILESIGNATUREDETECTOR_H
#define FILESIGNATUREDETECTOR_H

#include <QString>
#include <QByteArray>
#include <QList>

enum class MediaType {
    Unknown,
    Video,
    Audio,
    Image
};

struct FormatSignature {
    QString name;
    QString extension;
    MediaType type;
    QByteArray signature;
    int offset;
};

class FileSignatureDetector
{
public:
    static MediaType detectMediaType(const QString& filePath);
    static QString detectFormat(const QString& filePath);
    static QList<FormatSignature> getVideoFormats();
    static QList<FormatSignature> getAudioFormats();
    static QList<FormatSignature> getImageFormats();
    static QList<FormatSignature> getAllFormats();

private:
    static QList<FormatSignature> buildSignatureDatabase();
    static QByteArray readFileHeader(const QString& filePath, int size);
    static bool matchesSignature(const QByteArray& header, const FormatSignature& sig);
    static QList<FormatSignature> s_signatures;
};

#endif
