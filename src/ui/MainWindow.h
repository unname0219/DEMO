#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPropertyAnimation>
#include "core/PlayerController.h"
#include "core/FileSignatureDetector.h"

class HeaderBar;
class MediaViewer;
class PlaybackControls;
class ProgressBar;
class VolumeSlider;
class SpeedSelector;
class SettingsPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void openFile(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void openFileDialog();
    void toggleSettings();
    void toggleFullScreen();
    void onVolumeBoostRequested();
    void playAdjacentFile(int direction);
    void hideControlsAfterTimeout();
    void checkMousePosition();

private:
    void setupUI();
    void setupConnections();
    void setupShortcuts();
    void updateWindowTitle(const QString& fileName);
    void updateLayoutForMediaType(MediaType type);
    void repositionSettingsPanel();
    QString findAdjacentFile(const QString& currentPath, int direction);
    void updateResizeCursor(int x, int y);
    void showControls();
    void hideControls();
    void showFileInfoMenu(const QPoint& pos);
    void checkFileAssociations();
    void showNotification(const QString& title, const QString& message);

    HeaderBar* m_headerBar;
    MediaViewer* m_mediaViewer;
    PlaybackControls* m_playbackControls;
    ProgressBar* m_progressBar;
    VolumeSlider* m_volumeSlider;
    SpeedSelector* m_speedSelector;
    SettingsPanel* m_settingsPanel;
    PlayerController* m_playerController;

    QWidget* m_controlBar;
    QTimer* m_hideControlsTimer;
    QTimer* m_mouseCheckTimer;
    QPropertyAnimation* m_controlBarAnimation;
    QPropertyAnimation* m_progressBarAnimation;

    QString m_currentFile;
    bool m_isFullScreen;
    bool m_wasMaximizedBeforeFullScreen;
};

#endif
