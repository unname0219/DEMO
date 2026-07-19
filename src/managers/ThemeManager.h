#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QPalette>

enum class ThemeMode {
    System,
    Light,
    Dark
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();

    void init();
    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);
    QString currentStyleSheet() const;
    bool isDarkMode() const;

    QString primaryColor() const;
    QString backgroundColor() const;
    QString headerBackgroundColor() const;
    QString textColor() const;
    QString secondaryTextColor() const;
    QString borderColor() const;
    QString hoverColor() const;
    QString progressGradientStart() const;
    QString progressGradientEnd() const;

signals:
    void themeChanged();

private:
    ThemeManager();
    ~ThemeManager();

    static ThemeManager* s_instance;

    ThemeMode m_themeMode;
    QString m_styleSheet;

    void applyTheme();
    void loadStyleSheet();
    bool isSystemDark() const;
};

#endif
