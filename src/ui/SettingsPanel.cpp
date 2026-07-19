#include "ui/SettingsPanel.h"
#include "managers/ThemeManager.h"
#include "managers/FileAssociation.h"
#include "managers/PluginManager.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QComboBox>
#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
#include <QListWidget>
#include <QStackedWidget>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
    , m_navList(nullptr)
    , m_contentStack(nullptr)
    , m_titleBar(nullptr)
    , m_isDragging(false)
{
    setWindowTitle("Firefly Player - 设置");
    setModal(true);
    setMinimumSize(DPIAdapter::scaledSize(620), DPIAdapter::scaledSize(480));
    resize(DPIAdapter::scaledSize(620), DPIAdapter::scaledSize(520));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setupUI();
}

void SettingsPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor bg = ThemeManager::instance()->backgroundColor();
    painter.setBrush(QBrush(bg));
    QPen borderPen(ThemeManager::instance()->borderColor());
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 
                            DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
}

SettingsPanel::~SettingsPanel()
{
}

void SettingsPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8), 
                                   DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
    mainLayout->setSpacing(0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(DPIAdapter::scaledSize(36));
    m_titleBar->setCursor(Qt::SizeAllCursor);
    QHBoxLayout* titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(8), 0);
    titleLayout->setSpacing(DPIAdapter::scaledSize(8));

    QLabel* titleLabel = new QLabel("设置", m_titleBar);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(11));
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor()));
    titleLayout->addWidget(titleLabel);

    titleLayout->addStretch();

    QPushButton* closeBtn = new QPushButton(m_titleBar);
    closeBtn->setFixedSize(DPIAdapter::scaledSize(28), DPIAdapter::scaledSize(28));
    closeBtn->setIconSize(QSize(DPIAdapter::scaledSize(14), DPIAdapter::scaledSize(14)));
    closeBtn->setIcon(IconManager::instance()->icon("close"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip("关闭");
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(255,255,255,10); border-radius: 4px; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    titleLayout->addWidget(closeBtn);

    mainLayout->addWidget(m_titleBar);

    // 左侧导航 + 右侧内容区
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 左侧导航列表
    m_navList = new QListWidget(this);
    m_navList->setFixedWidth(DPIAdapter::scaledSize(110));
    m_navList->setSpacing(2);
    m_navList->setFrameShape(QFrame::NoFrame);
    m_navList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_navList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QStringList navItems;
    navItems << "外观" << "文件关联" << "格式支持包" << "播放设置" << "快捷键" << "关于";
    foreach (const QString& item, navItems) {
        m_navList->addItem(item);
    }
    m_navList->setCurrentRow(0);

    contentLayout->addWidget(m_navList);

    // 分隔线
    QFrame* vLine = new QFrame(this);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->borderColor()));
    contentLayout->addWidget(vLine);

    // 右侧内容区
    m_contentStack = new QStackedWidget(this);
    contentLayout->addWidget(m_contentStack, 1);

    mainLayout->addLayout(contentLayout, 1);

    QWidget* appearancePage = new QWidget();
    setupAppearancePage(appearancePage);
    m_contentStack->addWidget(appearancePage);

    QWidget* fileAssocPage = new QWidget();
    setupFileAssocPage(fileAssocPage);
    m_contentStack->addWidget(fileAssocPage);

    QWidget* pluginsPage = new QWidget();
    setupPluginsPage(pluginsPage);
    m_contentStack->addWidget(pluginsPage);

    QWidget* playbackPage = new QWidget();
    setupPlaybackPage(playbackPage);
    m_contentStack->addWidget(playbackPage);

    QWidget* shortcutsPage = new QWidget();
    setupShortcutsPage(shortcutsPage);
    m_contentStack->addWidget(shortcutsPage);

    QWidget* aboutPage = new QWidget();
    setupAboutPage(aboutPage);
    m_contentStack->addWidget(aboutPage);

    connect(m_navList, &QListWidget::currentRowChanged, m_contentStack, &QStackedWidget::setCurrentIndex);

    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &SettingsPanel::updateNavListStyle);
    updateNavListStyle();
}

void SettingsPanel::updateNavListStyle()
{
    if (!m_navList) return;
    m_navList->setStyleSheet(QString(
        "QListWidget { background: transparent; border: none; padding: %1px 0; }"
        "QListWidget::item { height: %2px; padding-left: %3px; color: %4; "
        "border-left: 2px solid transparent; }"
        "QListWidget::item:selected { background: rgba(0,212,170,15); color: %5; "
        "border-left: 2px solid %5; }"
        "QListWidget::item:hover { background: rgba(0,212,170,8); }"
    ).arg(
        QString::number(DPIAdapter::scaledSize(8)),
        QString::number(DPIAdapter::scaledSize(36)),
        QString::number(DPIAdapter::scaledSize(12)),
        ThemeManager::instance()->textColor(),
        ThemeManager::instance()->primaryColor()
    ));
    update();
}

void SettingsPanel::setupAppearancePage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(16));

    QGroupBox* themeGroup = new QGroupBox("主题模式", page);
    QVBoxLayout* themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(DPIAdapter::scaledSize(8));

    ThemeMode currentMode = ThemeManager::instance()->themeMode();

    QRadioButton* systemBtn = new QRadioButton("跟随系统", themeGroup);
    systemBtn->setChecked(currentMode == ThemeMode::System);
    connect(systemBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::System);
    });
    themeLayout->addWidget(systemBtn);

    QRadioButton* lightBtn = new QRadioButton("浅色模式", themeGroup);
    lightBtn->setChecked(currentMode == ThemeMode::Light);
    connect(lightBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Light);
    });
    themeLayout->addWidget(lightBtn);

    QRadioButton* darkBtn = new QRadioButton("深色模式", themeGroup);
    darkBtn->setChecked(currentMode == ThemeMode::Dark);
    connect(darkBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Dark);
    });
    themeLayout->addWidget(darkBtn);

    layout->addWidget(themeGroup);

    QGroupBox* imgGroup = new QGroupBox("图片缩放", page);
    QVBoxLayout* imgLayout = new QVBoxLayout(imgGroup);
    imgLayout->setSpacing(DPIAdapter::scaledSize(8));

    QSettings settings;
    bool smooth = settings.value("image/smoothScaling", true).toBool();

    QRadioButton* smoothBtn = new QRadioButton("平滑模式 (高质量插值)", imgGroup);
    smoothBtn->setChecked(smooth);
    connect(smoothBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", true);
            emit imageScalingChanged(true);
        }
    });
    imgLayout->addWidget(smoothBtn);

    QRadioButton* pixelBtn = new QRadioButton("像素模式 (显示像素点)", imgGroup);
    pixelBtn->setChecked(!smooth);
    connect(pixelBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", false);
            emit imageScalingChanged(false);
        }
    });
    imgLayout->addWidget(pixelBtn);

    layout->addWidget(imgGroup);
    layout->addStretch();
}

void SettingsPanel::setupFileAssocPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* desc = new QLabel("选择需要用 Firefly Player 打开的文件格式", page);
    desc->setWordWrap(true);
    QFont descFont = desc->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    desc->setFont(descFont);
    layout->addWidget(desc);

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(12));

    FileAssociation* fa = FileAssociation::instance();

    QGroupBox* videoGroup = new QGroupBox("视频格式", scrollContent);
    QVBoxLayout* videoLayout = new QVBoxLayout(videoGroup);
    videoLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList videoFormats = {"mp4", "mkv", "avi", "mov", "webm", "flv", "wmv", "ts", "3gp"};
    foreach (const QString& fmt, videoFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", videoGroup);
        cb->setChecked(fa->videoFormats().contains(fmt));
        cb->setProperty("format", fmt);
        videoLayout->addWidget(cb);
        m_videoCheckboxes.append(cb);
    }
    scrollLayout->addWidget(videoGroup);

    QGroupBox* audioGroup = new QGroupBox("音频格式", scrollContent);
    QVBoxLayout* audioLayout = new QVBoxLayout(audioGroup);
    audioLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList audioFormats = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus"};
    foreach (const QString& fmt, audioFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", audioGroup);
        cb->setChecked(fa->audioFormats().contains(fmt));
        cb->setProperty("format", fmt);
        audioLayout->addWidget(cb);
        m_audioCheckboxes.append(cb);
    }
    scrollLayout->addWidget(audioGroup);

    QGroupBox* imageGroup = new QGroupBox("图片格式", scrollContent);
    QVBoxLayout* imageLayout = new QVBoxLayout(imageGroup);
    imageLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList imageFormats = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "tiff", "svg"};
    foreach (const QString& fmt, imageFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", imageGroup);
        cb->setChecked(fa->imageFormats().contains(fmt));
        cb->setProperty("format", fmt);
        imageLayout->addWidget(cb);
        m_imageCheckboxes.append(cb);
    }
    scrollLayout->addWidget(imageGroup);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupPluginsPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* desc = new QLabel("管理可选的格式支持包，按需安装以支持更多格式", page);
    desc->setWordWrap(true);
    QFont descFont = desc->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    desc->setFont(descFont);
    layout->addWidget(desc);

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(8));

    PluginManager* pm = PluginManager::instance();
    QList<PluginInfo> plugins = pm->availablePlugins();

    foreach (const PluginInfo& plugin, plugins) {
        QFrame* pluginFrame = new QFrame(scrollContent);
        pluginFrame->setFrameShape(QFrame::StyledPanel);
        pluginFrame->setStyleSheet("QFrame { border: 1px solid #444444; border-radius: 6px; padding: 12px; }");
        QVBoxLayout* pluginLayout = new QVBoxLayout(pluginFrame);
        pluginLayout->setSpacing(DPIAdapter::scaledSize(6));

        QHBoxLayout* headerLayout = new QHBoxLayout();
        QLabel* nameLabel = new QLabel(plugin.name, pluginFrame);
        QFont nameFont = nameLabel->font();
        nameFont.setBold(true);
        nameFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
        nameLabel->setFont(nameFont);
        headerLayout->addWidget(nameLabel);
        headerLayout->addStretch();
        QLabel* sizeLabel = new QLabel(QString("%1 MB").arg(plugin.sizeMB), pluginFrame);
        QFont sizeFont = sizeLabel->font();
        sizeFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
        sizeLabel->setFont(sizeFont);
        headerLayout->addWidget(sizeLabel);
        pluginLayout->addLayout(headerLayout);

        QLabel* descLabel = new QLabel(plugin.description, pluginFrame);
        descLabel->setWordWrap(true);
        QFont pDescFont = descLabel->font();
        pDescFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
        descLabel->setFont(pDescFont);
        pluginLayout->addWidget(descLabel);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        QPushButton* actionBtn = new QPushButton(plugin.isInstalled ? "已安装" : "安装", pluginFrame);
        actionBtn->setEnabled(!plugin.isInstalled);
        actionBtn->setStyleSheet(plugin.isInstalled
            ? "QPushButton { background-color: #00D4AA; color: white; padding: 6px 16px; border-radius: 4px; }"
            : "QPushButton { background-color: transparent; border: 1px solid #00D4AA; color: #00D4AA; padding: 6px 16px; border-radius: 4px; }"
        );
        btnLayout->addWidget(actionBtn);
        pluginLayout->addLayout(btnLayout);

        scrollLayout->addWidget(pluginFrame);
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupPlaybackPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(16));

    QGroupBox* videoGroup = new QGroupBox("视频缩放", page);
    QVBoxLayout* videoLayout = new QVBoxLayout(videoGroup);
    videoLayout->setSpacing(DPIAdapter::scaledSize(8));

    QSettings settings;
    bool keepAspectRatio = settings.value("video/keepAspectRatio", true).toBool();

    QRadioButton* fitBtn = new QRadioButton("保持原宽高（完整显示）", videoGroup);
    fitBtn->setChecked(keepAspectRatio);
    connect(fitBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("video/keepAspectRatio", true);
            emit videoScalingModeChanged(Qt::KeepAspectRatio);
        }
    });
    videoLayout->addWidget(fitBtn);

    QRadioButton* stretchBtn = new QRadioButton("拉伸铺满窗口", videoGroup);
    stretchBtn->setChecked(!keepAspectRatio);
    connect(stretchBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("video/keepAspectRatio", false);
            emit videoScalingModeChanged(Qt::IgnoreAspectRatio);
        }
    });
    videoLayout->addWidget(stretchBtn);

    layout->addWidget(videoGroup);

    QGroupBox* speedGroup = new QGroupBox("默认播放速度", page);
    QVBoxLayout* speedLayout = new QVBoxLayout(speedGroup);
    speedLayout->setSpacing(DPIAdapter::scaledSize(8));

    QComboBox* speedCombo = new QComboBox(speedGroup);
    QStringList speeds = {"0.1x", "0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "1.75x", "2.0x", "5.0x"};
    speedCombo->addItems(speeds);
    speedCombo->setCurrentText("1.0x");
    speedLayout->addWidget(speedCombo);
    layout->addWidget(speedGroup);

    QGroupBox* speedModeGroup = new QGroupBox("倍速模式", page);
    QVBoxLayout* speedModeLayout = new QVBoxLayout(speedModeGroup);
    speedModeLayout->setSpacing(DPIAdapter::scaledSize(8));

    QSettings settings2;
    bool preservePitch = settings2.value("playback/preservePitch", true).toBool();

    QRadioButton* pitchBtn = new QRadioButton("不变调（推荐）", speedModeGroup);
    pitchBtn->setChecked(preservePitch);
    connect(pitchBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("playback/preservePitch", true);
            emit playbackSpeedModeChanged(true);
        }
    });
    speedModeLayout->addWidget(pitchBtn);

    QRadioButton* normalBtn = new QRadioButton("普通倍速（音调随速度变化）", speedModeGroup);
    normalBtn->setChecked(!preservePitch);
    connect(normalBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("playback/preservePitch", false);
            emit playbackSpeedModeChanged(false);
        }
    });
    speedModeLayout->addWidget(normalBtn);

    layout->addWidget(speedModeGroup);

    QGroupBox* volGroup = new QGroupBox("音量增强", page);
    QVBoxLayout* volLayout = new QVBoxLayout(volGroup);
    volLayout->setSpacing(DPIAdapter::scaledSize(8));

    QCheckBox* volBoostCb = new QCheckBox("启用音量增强（最高500%）", volGroup);
    volBoostCb->setChecked(false);
    volLayout->addWidget(volBoostCb);

    QLabel* volDesc = new QLabel("注意：过高音量可能损伤听力或设备", volGroup);
    QFont volDescFont = volDesc->font();
    volDescFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    volDesc->setFont(volDescFont);
    volLayout->addWidget(volDesc);

    layout->addWidget(volGroup);
    layout->addStretch();
}

void SettingsPanel::setupShortcutsPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(8));

    struct Shortcut { QString key; QString desc; };
    QList<Shortcut> shortcuts = {
        {"Space", "播放/暂停"},
        {"← / →", "快退/快进 5秒"},
        {"↑ / ↓", "音量增加/减少 10%"},
        {"M", "静音切换"},
        {"F / F11", "全屏切换"},
        {"Esc", "退出全屏"},
        {"Ctrl + + / -", "图片放大/缩小"},
        {"Ctrl + 0", "图片恢复原始大小"},
        {"← / → (图片)", "上一张/下一张"}
    };

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(4));

    foreach (const Shortcut& sc, shortcuts) {
        QFrame* row = new QFrame(scrollContent);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 6, 0, 6);

        QLabel* keyLabel = new QLabel(sc.key, row);
        keyLabel->setStyleSheet(
            "background: #3A3A3A; padding: 4px 14px; border-radius: 4px; font-family: monospace;"
        );
        keyLabel->setFixedWidth(DPIAdapter::scaledSize(140));
        keyLabel->setAlignment(Qt::AlignCenter);
        rowLayout->addWidget(keyLabel);

        QLabel* descLabel = new QLabel(sc.desc, row);
        rowLayout->addWidget(descLabel, 1);

        scrollLayout->addWidget(row);
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupAboutPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* logoLabel = new QLabel(page);
    logoLabel->setFixedSize(DPIAdapter::scaledSize(64), DPIAdapter::scaledSize(64));
    logoLabel->setPixmap(IconManager::instance()->icon("logo", false).pixmap(logoLabel->size()));
    logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(logoLabel);

    QLabel* nameLabel = new QLabel("Firefly Player", page);
    QFont nameFont = nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSizeF(DPIAdapter::scaledFontSize(14));
    nameLabel->setFont(nameFont);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor()));
    layout->addWidget(nameLabel);

    QLabel* versionLabel = new QLabel("版本 1.4.0", page);
    QFont versionFont = versionLabel->font();
    versionFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    versionLabel->setFont(versionFont);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor()));
    layout->addWidget(versionLabel);

    layout->addStretch();

    QLabel* descLabel = new QLabel("一个轻量级的媒体播放器，支持视频、音频和图片播放。", page);
    QFont descFont = descLabel->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    descLabel->setFont(descFont);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor()));
    layout->addWidget(descLabel);

    QLabel* authorLabel = new QLabel("© 2024 Firefly", page);
    QFont authorFont = authorLabel->font();
    authorFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    authorLabel->setFont(authorFont);
    authorLabel->setAlignment(Qt::AlignCenter);
    authorLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor()));
    layout->addWidget(authorLabel);
}

void SettingsPanel::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    refreshFileAssocPage();
}

void SettingsPanel::refreshFileAssocPage()
{
    FileAssociation* fa = FileAssociation::instance();
    fa->syncFromSystem();

    foreach (QCheckBox* cb, m_videoCheckboxes) {
        QString fmt = cb->property("format").toString();
        cb->setChecked(fa->videoFormats().contains(fmt));
    }
    foreach (QCheckBox* cb, m_audioCheckboxes) {
        QString fmt = cb->property("format").toString();
        cb->setChecked(fa->audioFormats().contains(fmt));
    }
    foreach (QCheckBox* cb, m_imageCheckboxes) {
        QString fmt = cb->property("format").toString();
        cb->setChecked(fa->imageFormats().contains(fmt));
    }
}

void SettingsPanel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_titleBar && m_titleBar->rect().contains(event->pos())) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
    QDialog::mousePressEvent(event);
}

void SettingsPanel::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragStartPosition);
        event->accept();
    }
    QDialog::mouseMoveEvent(event);
}

void SettingsPanel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
    QDialog::mouseReleaseEvent(event);
}
