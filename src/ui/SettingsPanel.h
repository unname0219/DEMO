#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QDialog>
#include <QTabWidget>

class SettingsPanel : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent = nullptr);
    ~SettingsPanel();

signals:
    void imageScalingChanged(bool smooth);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
    void setupAppearancePage(QWidget* page);
    void setupFileAssocPage(QWidget* page);
    void setupPluginsPage(QWidget* page);
    void setupPlaybackPage(QWidget* page);
    void setupShortcutsPage(QWidget* page);
    void setupAboutPage(QWidget* page);

    QTabWidget* m_tabWidget;
};

#endif
