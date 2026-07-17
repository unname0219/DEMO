#include "utils/FormatUtils.h"
#include <QStringList>

QString FormatUtils::formatTime(qint64 milliseconds)
{
    qint64 seconds = milliseconds / 1000;
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    qint64 secs = seconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
}

QString FormatUtils::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%1 KB").arg(bytes / 1024.0, 1, 'f', 1);
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 1, 'f', 1);
    } else {
        return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 1, 'f', 1);
    }
}

QString FormatUtils::formatSpeed(double speed)
{
    return QString("%1X").arg(speed, 0, 'f', speed == 1.0 ? 0 : 2).replace(".00", "");
}

QList<double> FormatUtils::availableSpeeds()
{
    return {0.1, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 5.0};
}
