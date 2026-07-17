#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "core/PlayerController.h"

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

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void openFileDialog();
    void openFile(const QString& filePath);
    void toggleSettings();
    void toggleFullScreen();
    void onVolumeBoostRequested();

private:
    void setupUI();
    void setupConnections();
    void setupShortcuts();
    void updateWindowTitle(const QString& fileName);

    HeaderBar* m_headerBar;
    MediaViewer* m_mediaViewer;
    PlaybackControls* m_playbackControls;
    ProgressBar* m_progressBar;
    VolumeSlider* m_volumeSlider;
    SpeedSelector* m_speedSelector;
    SettingsPanel* m_settingsPanel;
    PlayerController* m_playerController;

    QString m_currentFile;
    bool m_isFullScreen;
};

#endif
