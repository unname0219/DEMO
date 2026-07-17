#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent = nullptr);
    ~SettingsPanel();

private slots:
    void onCategoryChanged(int row);

private:
    void setupUI();
    void setupAppearancePage();
    void setupFileAssocPage();
    void setupPluginsPage();
    void setupPlaybackPage();
    void setupShortcutsPage();

    QListWidget* m_categoryList;
    QStackedWidget* m_contentStack;
};

#endif
