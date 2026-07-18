#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QDialog>
#include <QTabWidget>
#include <QMediaPlayer>
#include <QMouseEvent>

class SettingsPanel : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent = nullptr);
    ~SettingsPanel();

signals:
    void imageScalingChanged(bool smooth);
    void videoScalingModeChanged(Qt::AspectRatioMode mode);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setupUI();
    void setupAppearancePage(QWidget* page);
    void setupFileAssocPage(QWidget* page);
    void setupPluginsPage(QWidget* page);
    void setupPlaybackPage(QWidget* page);
    void setupShortcutsPage(QWidget* page);
    void setupAboutPage(QWidget* page);

    QTabWidget* m_tabWidget;
    QWidget* m_titleBar;
    bool m_isDragging;
    QPoint m_dragStartPosition;
};

#endif
