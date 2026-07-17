#include "ui/HeaderBar.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWindow>

HeaderBar::HeaderBar(QWidget* parent)
    : QWidget(parent)
    , m_logoLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_openFileBtn(nullptr)
    , m_minimizeBtn(nullptr)
    , m_maximizeBtn(nullptr)
    , m_closeBtn(nullptr)
    , m_title("Firefly")
{
    setFixedHeight(DPIAdapter::scaledSize(40));
    setMouseTracking(true);
    setupUI();
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(4), 0);
    layout->setSpacing(DPIAdapter::scaledSize(8));

    m_logoLabel = new QLabel("🔥 Firefly", this);
    QFont logoFont = m_logoLabel->font();
    logoFont.setPointSizeF(DPIAdapter::scaledFontSize(11));
    logoFont.setBold(true);
    m_logoLabel->setFont(logoFont);
    m_logoLabel->setStyleSheet("color: #00D4AA; background: transparent;");
    layout->addWidget(m_logoLabel);

    m_openFileBtn = new QPushButton("打开文件", this);
    QFont openFont = m_openFileBtn->font();
    openFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    m_openFileBtn->setFont(openFont);
    m_openFileBtn->setCursor(Qt::PointingHandCursor);
    m_openFileBtn->setStyleSheet(
        "QPushButton { padding: 4px 12px; border-radius: 4px; }"
        "QPushButton:hover { background-color: rgba(0,212,170,40); }"
    );
    connect(m_openFileBtn, &QPushButton::clicked, this, &HeaderBar::openFileClicked);
    layout->addWidget(m_openFileBtn);

    layout->addStretch();

    m_titleLabel = new QLabel(m_title, this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("background: transparent;");
    layout->addWidget(m_titleLabel, 1);

    layout->addStretch();

    m_minimizeBtn = new QPushButton("—", this);
    m_minimizeBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(28));
    QFont btnFont = m_minimizeBtn->font();
    btnFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    m_minimizeBtn->setFont(btnFont);
    m_minimizeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeBtn, &QPushButton::clicked, this, &HeaderBar::minimizeClicked);
    layout->addWidget(m_minimizeBtn);

    m_maximizeBtn = new QPushButton("□", this);
    m_maximizeBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(28));
    m_maximizeBtn->setFont(btnFont);
    m_maximizeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &HeaderBar::maximizeClicked);
    layout->addWidget(m_maximizeBtn);

    m_closeBtn = new QPushButton("×", this);
    m_closeBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(28));
    m_closeBtn->setFont(btnFont);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton:hover { background-color: #E81123; color: white; }"
    );
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
    int maxWidth = width() - DPIAdapter::scaledSize(220);
    if (maxWidth < DPIAdapter::scaledSize(80)) maxWidth = DPIAdapter::scaledSize(80);
    QFontMetrics fm(m_titleLabel->font());
    QString elided = fm.elidedText(m_title, Qt::ElideMiddle, maxWidth);
    m_titleLabel->setText(elided);
}

void HeaderBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateTitleElided();
}

// 无边框窗口：在标题栏按下左键拖动整个窗口
void HeaderBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget* w = window();
        if (w && w->windowHandle()) {
            w->windowHandle()->startSystemMove();
        }
    }
    QWidget::mousePressEvent(event);
}

// 双击标题栏最大化/还原
void HeaderBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit maximizeClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}
