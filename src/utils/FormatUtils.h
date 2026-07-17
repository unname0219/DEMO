#ifndef FORMATUTILS_H
#define FORMATUTILS_H

#include <QString>
#include <QList>

class FormatUtils
{
public:
    static QString formatTime(qint64 milliseconds);
    static QString formatFileSize(qint64 bytes);
    static QString formatSpeed(double speed);
    static QList<double> availableSpeeds();
};

#endif
