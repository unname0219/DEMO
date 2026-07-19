#include "ui/SpeedSelector.h"
#include "managers/DPIAdapter.h"
#include "managers/ThemeManager.h"
#include "utils/FormatUtils.h"
#include <QHBoxLayout>

SpeedSelector::SpeedSelector(QWidget* parent)
    : QWidget(parent)
    , m_comboBox(nullptr)
    , m_currentSpeed(1.0)
{
    setFixedHeight(DPIAdapter::scaledSize(36));
    setupUI();
}

SpeedSelector::~SpeedSelector()
{
}

void SpeedSelector::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_comboBox = new QComboBox(this);
    m_comboBox->setFixedHeight(DPIAdapter::scaledSize(32));
    m_comboBox->setMinimumWidth(DPIAdapter::scaledSize(80));
    QString bg = ThemeManager::instance()->backgroundColor();
    QString text = ThemeManager::instance()->textColor();
    QString border = ThemeManager::instance()->borderColor();
    QString hover = ThemeManager::instance()->hoverColor();
    m_comboBox->setStyleSheet(
        QString("QComboBox { border: none; background: %1; color: %2; padding: 0 8px; }"
                "QComboBox::drop-down { border: none; }"
                "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid %2; }"
                "QComboBox QAbstractItemView { background: %1; color: %2; border: 1px solid %3; selection-background-color: %4; }")
        .arg(bg).arg(text).arg(border).arg(hover)
    );

    QList<double> speeds = FormatUtils::availableSpeeds();
    for (double speed : speeds) {
        m_comboBox->addItem(FormatUtils::formatSpeed(speed), speed);
    }

    int defaultIndex = speeds.indexOf(1.0);
    if (defaultIndex >= 0) {
        m_comboBox->setCurrentIndex(defaultIndex);
    }

    connect(m_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpeedSelector::onCurrentIndexChanged);

    layout->addWidget(m_comboBox);
}

void SpeedSelector::onCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    double speed = m_comboBox->currentData().toDouble();
    if (speed != m_currentSpeed) {
        m_currentSpeed = speed;
        emit speedChanged(speed);
    }
}

void SpeedSelector::onSpeedChanged(double speed)
{
    m_currentSpeed = speed;
    int index = m_comboBox->findData(speed);
    if (index >= 0 && index != m_comboBox->currentIndex()) {
        m_comboBox->blockSignals(true);
        m_comboBox->setCurrentIndex(index);
        m_comboBox->blockSignals(false);
    }
}
