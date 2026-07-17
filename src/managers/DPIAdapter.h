#ifndef DPIADAPTER_H
#define DPIADAPTER_H

#include <QObject>
#include <QWidget>

class DPIAdapter : public QObject
{
    Q_OBJECT

public:
    static void applyGlobalSettings();
    static double dpiScale();
    static double fontScale();
    static int scaledSize(int baseSize);
    static int scaledFontSize(int basePt);
    static void adjustWidgetFont(QWidget* widget, int basePt = 13);
    static int minimumFontSize();

private:
    static double s_dpiScale;
    static double s_fontScale;
    static void calculateScales();
};

#endif
