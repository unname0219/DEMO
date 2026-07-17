#include "managers/ThemeManager.h"
#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>
#include <QPalette>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager();
    }
    return s_instance;
}

ThemeManager::ThemeManager()
    : QObject()
    , m_themeMode(ThemeMode::System)
{
}

ThemeManager::~ThemeManager()
{
}

void ThemeManager::init()
{
    QSettings settings;
    int mode = settings.value("themeMode", static_cast<int>(ThemeMode::System)).toInt();
    m_themeMode = static_cast<ThemeMode>(mode);

    applyTheme();
}

ThemeMode ThemeManager::themeMode() const
{
    return m_themeMode;
}

void ThemeManager::setThemeMode(ThemeMode mode)
{
    if (m_themeMode == mode) return;

    m_themeMode = mode;
    QSettings settings;
    settings.setValue("themeMode", static_cast<int>(mode));

    applyTheme();
}

QString ThemeManager::currentStyleSheet() const
{
    return m_styleSheet;
}

bool ThemeManager::isDarkMode() const
{
    if (m_themeMode == ThemeMode::System) {
        return isSystemDark();
    }
    return m_themeMode == ThemeMode::Dark;
}

bool ThemeManager::isSystemDark() const
{
    QPalette palette = QGuiApplication::palette();
    QColor textColor = palette.color(QPalette::WindowText);
    QColor windowColor = palette.color(QPalette::Window);
    return textColor.lightness() > windowColor.lightness();
}

QString ThemeManager::primaryColor() const
{
    return "#00D4AA";
}

QString ThemeManager::backgroundColor() const
{
    return isDarkMode() ? "#2D2D2D" : "#FAF8F5";
}

QString ThemeManager::textColor() const
{
    return isDarkMode() ? "#E0E0E0" : "#333333";
}

QString ThemeManager::secondaryTextColor() const
{
    return isDarkMode() ? "#888888" : "#666666";
}

QString ThemeManager::borderColor() const
{
    return isDarkMode() ? "#444444" : "#E0E0E0";
}

QString ThemeManager::hoverColor() const
{
    return isDarkMode() ? "#3A3A3A" : "#F0EDE8";
}

QString ThemeManager::progressGradientStart() const
{
    return "#00D4AA";
}

QString ThemeManager::progressGradientEnd() const
{
    return "#00FFC8";
}

void ThemeManager::applyTheme()
{
    loadStyleSheet();
    qApp->setStyleSheet(m_styleSheet);
    emit themeChanged();
}

void ThemeManager::loadStyleSheet()
{
    bool dark = isDarkMode();
    QString bg = dark ? "#2D2D2D" : "#FAF8F5";
    QString text = dark ? "#E0E0E0" : "#333333";
    QString secondary = dark ? "#888888" : "#666666";
    QString border = dark ? "#444444" : "#E0E0E0";
    QString hover = dark ? "#3A3A3A" : "#F0EDE8";
    QString headerBg = dark ? "#1F1F1F" : "#FFFFFF";
    QString controlBg = dark ? "#252525" : "#FFFFFF";
    QString primary = "#00D4AA";
    QString primaryHover = "#00F0C0";
    QString progressStart = "#00D4AA";
    QString progressEnd = "#00FFC8";

    m_styleSheet = QString(
        "QWidget { background-color: %1; color: %2; font-size: 13pt; }"
        "QMainWindow { background-color: %1; }"
        "QHeaderBar { background-color: %6; border-bottom: 1px solid %4; }"
        "QPushButton { background-color: transparent; border: none; padding: 6px; border-radius: 4px; color: %2; }"
        "QPushButton:hover { background-color: %5; }"
        "QPushButton:pressed { background-color: %4; }"
        "QLabel { color: %2; background: transparent; }"
        "QSlider::groove:horizontal { height: 4px; background: %4; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %9, stop:1 %10); border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 12px; height: 12px; margin: -4px 0; background: %7; border-radius: 6px; }"
        "QSlider::groove:vertical { width: 4px; background: %4; border-radius: 2px; }"
        "QSlider::sub-page:vertical { background: qlineargradient(x1:0, y1:1, x2:0, y2:0, stop:0 %9, stop:1 %10); border-radius: 2px; }"
        "QSlider::handle:vertical { width: 12px; height: 12px; margin: 0 -4px; background: %7; border-radius: 6px; }"
        "QComboBox { background-color: %6; border: 1px solid %4; border-radius: 4px; padding: 4px 8px; color: %2; }"
        "QComboBox:hover { border-color: %7; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid %3; }"
        "QComboBox QAbstractItemView { background-color: %6; border: 1px solid %4; selection-background-color: %5; color: %2; }"
        "QMenu { background-color: %6; border: 1px solid %4; color: %2; }"
        "QMenu::item { padding: 6px 20px; }"
        "QMenu::item:selected { background-color: %5; }"
        "QCheckBox { spacing: 8px; color: %2; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 2px solid %4; border-radius: 3px; background: %1; }"
        "QCheckBox::indicator:checked { background: %7; border-color: %7; }"
        "QScrollBar:vertical { background: %1; width: 8px; }"
        "QScrollBar::handle:vertical { background: %4; border-radius: 4px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: %3; }"
        "QScrollBar:horizontal { background: %1; height: 8px; }"
        "QScrollBar::handle:horizontal { background: %4; border-radius: 4px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: %3; }"
        "QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }"
        "QDialog { background-color: %1; }"
        "QGroupBox { border: 1px solid %4; border-radius: 6px; margin-top: 12px; padding-top: 12px; color: %2; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
    ).arg(bg, text, secondary, border, hover, headerBg, primary, controlBg, progressStart, progressEnd);
}
