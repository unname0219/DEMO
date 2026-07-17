#include "managers/DPIAdapter.h"
#include <QApplication>
#include <QScreen>
#include <QFont>

double DPIAdapter::s_dpiScale = 1.0;
double DPIAdapter::s_fontScale = 1.0;

void DPIAdapter::applyGlobalSettings()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif

    calculateScales();

    QFont font = QApplication::font();
    font.setPointSizeF(scaledFontSize(10));
    QApplication::setFont(font);
}

void DPIAdapter::calculateScales()
{
    QScreen* screen = QApplication::primaryScreen();
    if (!screen) {
        s_dpiScale = 1.0;
        s_fontScale = 1.0;
        return;
    }

    qreal logicalDpi = screen->logicalDotsPerInch();
    qreal dpiRatio = logicalDpi / 96.0;

    if (dpiRatio <= 1.0) {
        s_dpiScale = 1.0;
        s_fontScale = 1.0;
    } else if (dpiRatio <= 1.5) {
        s_dpiScale = 1.25;
        s_fontScale = 1.15;
    } else if (dpiRatio <= 2.0) {
        s_dpiScale = 1.5;
        s_fontScale = 1.3;
    } else if (dpiRatio <= 2.5) {
        s_dpiScale = 2.0;
        s_fontScale = 1.5;
    } else {
        s_dpiScale = 2.5;
        s_fontScale = 1.7;
    }
}

double DPIAdapter::dpiScale()
{
    return s_dpiScale;
}

double DPIAdapter::fontScale()
{
    return s_fontScale;
}

int DPIAdapter::scaledSize(int baseSize)
{
    return qRound(baseSize * s_dpiScale);
}

int DPIAdapter::scaledFontSize(int basePt)
{
    return qMax(minimumFontSize(), qRound(basePt * s_fontScale));
}

void DPIAdapter::adjustWidgetFont(QWidget* widget, int basePt)
{
    if (!widget) return;
    QFont font = widget->font();
    font.setPointSizeF(scaledFontSize(basePt));
    widget->setFont(font);
}

int DPIAdapter::minimumFontSize()
{
    return 11;
}
