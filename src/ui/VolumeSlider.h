#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>

class VolumeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeSlider(QWidget* parent = nullptr);
    ~VolumeSlider();

signals:
    void volumeChanged(int volume);
    void muteToggled();

public slots:
    void onVolumeChanged(int volume);
    void onMutedChanged(bool muted);

private slots:
    void onSliderValueChanged(int value);
    void onMuteBtnClicked();

private:
    void setupUI();
    void updateMuteIcon();

    QPushButton* m_muteBtn;
    QSlider* m_volumeSlider;
    int m_volume;
    bool m_isMuted;
};

#endif
