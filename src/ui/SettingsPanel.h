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

signals:
    // 图片缩放模式改变时立即发出，由 ImageViewer 实时响应（无需重启）
    void imageScalingChanged(bool smooth);

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
