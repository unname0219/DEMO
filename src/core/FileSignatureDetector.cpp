#include "core/FileSignatureDetector.h"
#include <QFile>
#include <QFileInfo>

QList<FormatSignature> FileSignatureDetector::s_signatures;

QList<FormatSignature> FileSignatureDetector::buildSignatureDatabase()
{
    QList<FormatSignature> db;

    db.append({"MPEG-4", "mp4", MediaType::Video, QByteArray::fromHex("66747970"), 4});
    db.append({"QuickTime", "mov", MediaType::Video, QByteArray::fromHex("6D6F6F76"), 4});
    db.append({"MKV/WebM", "mkv", MediaType::Video, QByteArray::fromHex("1A45DFA3"), 0});
    db.append({"AVI", "avi", MediaType::Video, QByteArray::fromHex("52494646"), 0});
    db.append({"FLV", "flv", MediaType::Video, QByteArray::fromHex("464C56"), 0});
    db.append({"MPEG-TS", "ts", MediaType::Video, QByteArray::fromHex("47"), 0});
    db.append({"WMV", "wmv", MediaType::Video, QByteArray::fromHex("3026B275"), 0});

    db.append({"MP3", "mp3", MediaType::Audio, QByteArray::fromHex("FFFB"), 0});
    db.append({"MP3 ID3", "mp3", MediaType::Audio, QByteArray::fromHex("494433"), 0});
    db.append({"WAV", "wav", MediaType::Audio, QByteArray::fromHex("52494646"), 0});
    db.append({"FLAC", "flac", MediaType::Audio, QByteArray::fromHex("664C6143"), 0});
    db.append({"OGG", "ogg", MediaType::Audio, QByteArray::fromHex("4F676753"), 0});
    db.append({"AAC", "aac", MediaType::Audio, QByteArray::fromHex("FFF1"), 0});
    db.append({"WMA", "wma", MediaType::Audio, QByteArray::fromHex("3026B275"), 0});
    db.append({"M4A", "m4a", MediaType::Audio, QByteArray::fromHex("667479704D344120"), 4});

    db.append({"JPEG", "jpg", MediaType::Image, QByteArray::fromHex("FFD8FF"), 0});
    db.append({"PNG", "png", MediaType::Image, QByteArray::fromHex("89504E470D0A1A0A"), 0});
    db.append({"GIF", "gif", MediaType::Image, QByteArray::fromHex("47494638"), 0});
    db.append({"BMP", "bmp", MediaType::Image, QByteArray::fromHex("424D"), 0});
    db.append({"WebP", "webp", MediaType::Image, QByteArray::fromHex("52494646"), 0});
    db.append({"TIFF", "tiff", MediaType::Image, QByteArray::fromHex("49492A00"), 0});
    db.append({"TIFF BE", "tiff", MediaType::Image, QByteArray::fromHex("4D4D002A"), 0});
    db.append({"SVG", "svg", MediaType::Image, QByteArray("<?xml"), 0});
    db.append({"ICO", "ico", MediaType::Image, QByteArray::fromHex("00000100"), 0});

    return db;
}

QByteArray FileSignatureDetector::readFileHeader(const QString& filePath, int size)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    QByteArray header = file.read(size);
    file.close();
    return header;
}

bool FileSignatureDetector::matchesSignature(const QByteArray& header, const FormatSignature& sig)
{
    if (sig.offset + sig.signature.size() > header.size()) {
        return false;
    }
    return header.mid(sig.offset, sig.signature.size()) == sig.signature;
}

MediaType FileSignatureDetector::detectMediaType(const QString& filePath)
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }

    QByteArray header = readFileHeader(filePath, 64);
    if (header.isEmpty()) {
        return MediaType::Unknown;
    }

    for (const auto& sig : s_signatures) {
        if (matchesSignature(header, sig)) {
            return sig.type;
        }
    }

    QFileInfo fi(filePath);
    QString ext = fi.suffix().toLower();
    QStringList videoExts = {"mp4", "mkv", "avi", "mov", "webm", "flv", "wmv", "ts", "m2ts", "3gp"};
    QStringList audioExts = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus", "alac"};
    QStringList imageExts = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "tiff", "svg", "heic", "avif"};

    if (videoExts.contains(ext)) return MediaType::Video;
    if (audioExts.contains(ext)) return MediaType::Audio;
    if (imageExts.contains(ext)) return MediaType::Image;

    return MediaType::Unknown;
}

QString FileSignatureDetector::detectFormat(const QString& filePath)
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }

    QByteArray header = readFileHeader(filePath, 64);
    if (header.isEmpty()) {
        return QString();
    }

    for (const auto& sig : s_signatures) {
        if (matchesSignature(header, sig)) {
            return sig.name;
        }
    }

    return QString();
}

QList<FormatSignature> FileSignatureDetector::getVideoFormats()
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }
    QList<FormatSignature> result;
    for (const auto& sig : s_signatures) {
        if (sig.type == MediaType::Video) {
            result.append(sig);
        }
    }
    return result;
}

QList<FormatSignature> FileSignatureDetector::getAudioFormats()
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }
    QList<FormatSignature> result;
    for (const auto& sig : s_signatures) {
        if (sig.type == MediaType::Audio) {
            result.append(sig);
        }
    }
    return result;
}

QList<FormatSignature> FileSignatureDetector::getImageFormats()
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }
    QList<FormatSignature> result;
    for (const auto& sig : s_signatures) {
        if (sig.type == MediaType::Image) {
            result.append(sig);
        }
    }
    return result;
}

QList<FormatSignature> FileSignatureDetector::getAllFormats()
{
    if (s_signatures.isEmpty()) {
        s_signatures = buildSignatureDatabase();
    }
    return s_signatures;
}
