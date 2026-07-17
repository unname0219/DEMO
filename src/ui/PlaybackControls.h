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

public slots:
    void onPlaybackStateChanged(PlaybackState state);

private:
    void setupUI();

    QPushButton* m_playPauseBtn;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    PlaybackState m_state;
};

#endif
