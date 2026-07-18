#ifndef PLAYBACKCONTROLS_H
#define PLAYBACKCONTROLS_H

#include <QWidget>
#include <QPushButton>
#include "core/PlayerController.h"

class PlaybackControls : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackControls(QWidget* parent = nullptr);
    ~PlaybackControls();

signals:
    void playToggled();
    void previousClicked();
    void nextClicked();
    void rewindClicked();
    void forwardClicked();

public slots:
    void onPlaybackStateChanged(PlaybackState state);

private slots:
    void refreshIcons();

private:
    void setupUI();

    QPushButton* m_playPauseBtn;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    QPushButton* m_rewindBtn;
    QPushButton* m_forwardBtn;
    PlaybackState m_state;
};

#endif
