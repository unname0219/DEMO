#ifndef SPEEDSELECTOR_H
#define SPEEDSELECTOR_H

#include <QWidget>
#include <QComboBox>

class SpeedSelector : public QWidget
{
    Q_OBJECT

public:
    explicit SpeedSelector(QWidget* parent = nullptr);
    ~SpeedSelector();

signals:
    void speedChanged(double speed);

public slots:
    void onSpeedChanged(double speed);

private slots:
    void onCurrentIndexChanged(int index);
    void updateStyle();

private:
    void setupUI();

    QComboBox* m_comboBox;
    double m_currentSpeed;
};

#endif
