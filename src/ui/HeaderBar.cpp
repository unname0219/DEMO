#include "ui/HeaderBar.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include "managers/ThemeManager.h"
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWindow>
#include <QIcon>

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
    refreshIcons();
}

HeaderBar::~HeaderBar()
{
}

void HeaderBar::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(4), 0);
    layout->setSpacing(DPIAdapter::scaledSize(8));

    // Logo 图标（保留原色，不染色）
    int logoSize = DPIAdapter::scaledSize(24);
    m_logoLabel = new QLabel(this);
    m_logoLabel->setFixedSize(logoSize, logoSize);
    m_logoLabel->setScaledContents(true);
    layout->addWidget(m_logoLabel);

    // 品牌名
    QLabel* brandLabel = new QLabel("Firefly", this);
    QFont brandFont = brandLabel->font();
    brandFont.setPointSizeF(DPIAdapter::scaledFontSize(11));
    brandFont.setBold(true);
    brandLabel->setFont(brandFont);
    brandLabel->setStyleSheet("color: #00D4AA; background: transparent;");
    layout->addWidget(brandLabel);

    m_openFileBtn = new QPushButton(this);
    int btnIconSize = DPIAdapter::scaledSize(16);
    m_openFileBtn->setIconSize(QSize(btnIconSize, btnIconSize));
    m_openFileBtn->setText("打开文件");
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

    // 窗口控制按钮（图标化）
    int winBtnW = DPIAdapter::scaledSize(34);
    int winBtnH = DPIAdapter::scaledSize(28);
    int winIconSize = DPIAdapter::scaledSize(14);

    m_minimizeBtn = new QPushButton(this);
    m_minimizeBtn->setFixedSize(winBtnW, winBtnH);
    m_minimizeBtn->setIconSize(QSize(winIconSize, winIconSize));
    m_minimizeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeBtn, &QPushButton::clicked, this, &HeaderBar::minimizeClicked);
    layout->addWidget(m_minimizeBtn);

    m_maximizeBtn = new QPushButton(this);
    m_maximizeBtn->setFixedSize(winBtnW, winBtnH);
    m_maximizeBtn->setIconSize(QSize(winIconSize, winIconSize));
    m_maximizeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &HeaderBar::maximizeClicked);
    layout->addWidget(m_maximizeBtn);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(winBtnW, winBtnH);
    m_closeBtn->setIconSize(QSize(winIconSize, winIconSize));
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton:hover { background-color: #E81123; color: white; }"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &HeaderBar::closeClicked);
    layout->addWidget(m_closeBtn);

    // 主题变化时刷新图标
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &HeaderBar::refreshIcons);
}

void HeaderBar::refreshIcons()
{
    // Logo 保留原色
    m_logoLabel->setPixmap(IconManager::instance()->icon("logo", false).pixmap(m_logoLabel->size()));
    m_openFileBtn->setIcon(IconManager::instance()->icon("open-file"));
    m_minimizeBtn->setIcon(IconManager::instance()->icon("minimize"));
    m_maximizeBtn->setIcon(IconManager::instance()->icon("maximize"));
    m_closeBtn->setIcon(IconManager::instance()->icon("close"));
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

void HeaderBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit maximizeClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}
