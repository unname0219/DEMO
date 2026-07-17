#include "ui/HeaderBar.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QResizeEvent>

HeaderBar::HeaderBar(QWidget* parent)
    : QWidget(parent)
    , m_logoLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_minimizeBtn(nullptr)
    , m_maximizeBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_title("Firefly")
{
    setFixedHeight(DPIAdapter::scaledSize(40));
    setupUI();
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(DPIAdapter::scaledSize(12), 0, 0, 0);
    layout->setSpacing(DPIAdapter::scaledSize(8));

    m_logoLabel = new QLabel("🔥 Firefly", this);
    QFont logoFont = m_logoLabel->font();
    logoFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    logoFont.setBold(true);
    m_logoLabel->setFont(logoFont);
    m_logoLabel->setStyleSheet("color: #00D4AA;");
    layout->addWidget(m_logoLabel);

    layout->addStretch();

    m_titleLabel = new QLabel(m_title, this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(11));
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_titleLabel, 1);

    layout->addStretch();

    m_minimizeBtn = new QPushButton("—", this);
    m_minimizeBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    QFont btnFont = m_minimizeBtn->font();
    btnFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    m_minimizeBtn->setFont(btnFont);
    connect(m_minimizeBtn, &QPushButton::clicked, this, &HeaderBar::minimizeClicked);
    layout->addWidget(m_minimizeBtn);

    m_maximizeBtn = new QPushButton("□", this);
    m_maximizeBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    m_maximizeBtn->setFont(btnFont);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &HeaderBar::maximizeClicked);
    layout->addWidget(m_maximizeBtn);

    m_closeBtn = new QPushButton("×", this);
    m_closeBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    m_closeBtn->setFont(btnFont);
    m_closeBtn->setStyleSheet("QPushButton:hover { background-color: #E81123; color: white; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &HeaderBar::closeClicked);
    layout->addWidget(m_closeBtn);
}

void HeaderBar::setTitle(const QString& title)
{
    m_title = title;
    updateTitleElided();
}

void HeaderBar::updateTitleElided()
{
    if (!m_titleLabel) return;
    int maxWidth = width() - DPIAdapter::scaledSize(200);
    if (maxWidth < DPIAdapter::scaledSize(100)) maxWidth = DPIAdapter::scaledSize(100);
    QFontMetrics fm(m_titleLabel->font());
    QString elided = fm.elidedText(m_title, Qt::ElideMiddle, maxWidth);
    m_titleLabel->setText(elided);
}

void HeaderBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateTitleElided();
}
