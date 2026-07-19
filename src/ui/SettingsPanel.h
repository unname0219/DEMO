#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QDialog>
#include <QTabWidget>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QShowEvent>
#include <QStackedWidget>
#include <QListWidget>
#include <QCheckBox>

class SettingsPanel : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent = nullptr);
    ~SettingsPanel();

signals:
    void imageScalingChanged(bool smooth);
    void videoScalingModeChanged(Qt::AspectRatioMode mode);
    void playbackSpeedModeChanged(bool preservePitch);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void setupAppearancePage(QWidget* page);
    void setupFileAssocPage(QWidget* page);
    void setupPluginsPage(QWidget* page);
    void setupPlaybackPage(QWidget* page);
    void setupShortcutsPage(QWidget* page);
    void setupAboutPage(QWidget* page);
    void refreshFileAssocPage();

    QTabWidget* m_tabWidget;
    QListWidget* m_navList;
    QStackedWidget* m_contentStack;
    QWidget* m_titleBar;
    QList<QCheckBox*> m_videoCheckboxes;
    QList<QCheckBox*> m_audioCheckboxes;
    QList<QCheckBox*> m_imageCheckboxes;
    bool m_isDragging;
    QPoint m_dragStartPosition;
};

#endif
