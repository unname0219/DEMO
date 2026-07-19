#include "managers/ThemeManager.h"
#include <QApplication>
#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>
#include <QPalette>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

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

    // 监听系统主题变化（Qt 6.5+），跟随系统模式时实时切换
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QStyleHints* hints = QGuiApplication::styleHints();
    if (hints) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme scheme) {
            Q_UNUSED(scheme);
            if (m_themeMode == ThemeMode::System) {
                applyTheme();
                emit themeChanged();
            }
        });
    }
#endif
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
    // 1) 优先使用 Qt 6.5+ 的官方 API（最可靠，跨平台）
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QStyleHints* hints = QGuiApplication::styleHints();
    if (hints) {
        Qt::ColorScheme scheme = hints->colorScheme();
        if (scheme != Qt::ColorScheme::Unknown) {
            return scheme == Qt::ColorScheme::Dark;
        }
    }
#endif

    // 2) Windows 注册表兜底（最可靠，直接读取系统设置）
#ifdef Q_OS_WIN
    HKEY hKey;
    DWORD value = 1;
    DWORD size = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(&value), &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            // AppsUseLightTheme: 1=浅色, 0=深色
            return value == 0;
        }
        RegCloseKey(hKey);
    }
#endif

    // 3) 最终兜底：基于调色板判断
    QPalette palette = QGuiApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);
    return windowColor.lightness() < 128;
}

QString ThemeManager::primaryColor() const
{
    return "#00D4AA";
}

QString ThemeManager::backgroundColor() const
{
    return isDarkMode() ? "#2D2D2D" : "#FAF7F0";
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
    QString bg = dark ? "#2D2D2D" : "#FAF7F0";
    QString text = dark ? "#E0E0E0" : "#333333";
    QString secondary = dark ? "#888888" : "#666666";
    QString border = dark ? "#444444" : "#E8E3D8";
    QString hover = dark ? "#3A3A3A" : "#F0ECE2";
    QString headerBg = dark ? "#1F1F1F" : "#FFFDF8";
    QString controlBg = dark ? "#252525" : "#FFFDF8";
    QString primary = "#00D4AA";
    QString progressStart = "#00D4AA";
    QString progressEnd = "#00FFC8";

    // 注意：不在 QSS 中写死 font-size，字体大小由 DPIAdapter 统一管理，
    // 避免与 DPI 缩放叠加导致字体过大。
    m_styleSheet = QString(
        "QWidget { background-color: %1; color: %2; font-size: 13pt; }"
        "QMainWindow { background-color: transparent; }"
        "QHeaderBar { background-color: %6; border-bottom: 1px solid %4; }"
        "QPushButton { background-color: transparent; border: none; padding: 6px; border-radius: 4px; color: %2; }"
        "QPushButton:hover { background-color: %5; }"
        "QPushButton:pressed { background-color: %4; }"
        "QLabel { color: %2; background: transparent; }"
        "QListWidget { background-color: %6; border: none; outline: none; }"
        "QListWidget::item { padding: 8px 12px; border-radius: 4px; color: %2; }"
        "QListWidget::item:selected { background-color: %7; color: white; }"
        "QListWidget::item:hover { background-color: %5; }"
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
        "QRadioButton { spacing: 8px; color: %2; }"
        "QRadioButton::indicator { width: 14px; height: 14px; border: 2px solid %4; border-radius: 7px; background: %1; }"
        "QRadioButton::indicator:checked { background: %7; border-color: %7; }"
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
        "QScrollArea { border: none; background: transparent; }"
        "QFrame[role=\"separator\"] { background: %4; }"
    ).arg(bg, text, secondary, border, hover, headerBg, primary, controlBg, progressStart, progressEnd);
}
