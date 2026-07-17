#include "ui/FileAssocDialog.h"
#include "managers/DPIAdapter.h"
#include "managers/FileAssociation.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QCheckBox>

FileAssocDialog::FileAssocDialog(QWidget* parent)
    : QDialog(parent)
    , m_videoGroup(nullptr)
    , m_audioGroup(nullptr)
    , m_imageGroup(nullptr)
{
    setWindowTitle("欢迎使用 Firefly Player");
    setMinimumSize(DPIAdapter::scaledSize(600), DPIAdapter::scaledSize(500));
    resize(DPIAdapter::scaledSize(650), DPIAdapter::scaledSize(550));
    setupUI();
}

FileAssocDialog::~FileAssocDialog()
{
}

void FileAssocDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(DPIAdapter::scaledSize(24), DPIAdapter::scaledSize(24),
                                    DPIAdapter::scaledSize(24), DPIAdapter::scaledSize(24));
    mainLayout->setSpacing(DPIAdapter::scaledSize(16));

    QLabel* title = new QLabel("欢迎使用 Firefly Player", this);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(20));
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #00D4AA;");
    mainLayout->addWidget(title);

    QLabel* subtitle = new QLabel("选择需要用 Firefly Player 打开的文件格式", this);
    subtitle->setStyleSheet("color: #888888;");
    mainLayout->addWidget(subtitle);

    QLabel* hint = new QLabel("（已为您自动勾选常见格式，您可以根据需要自定义）", this);
    hint->setStyleSheet("color: #888888; font-size: 10pt;");
    mainLayout->addWidget(hint);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(16));

    setupVideoSection(scrollContent);
    setupAudioSection(scrollContent);
    setupImageSection(scrollContent);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText("开始使用");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("跳过");
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void FileAssocDialog::setupVideoSection(QWidget* parent)
{
    m_videoGroup = new QGroupBox("视频格式", parent);
    QVBoxLayout* layout = new QVBoxLayout(m_videoGroup);

    QStringList formats = {"mp4", "mkv", "avi", "mov", "webm", "flv", "wmv", "ts", "m2ts", "3gp"};
    QStringList defaultFmts = {"mp4", "mkv", "avi", "mov", "webm", "flv"};

    QHBoxLayout* rowLayout = nullptr;
    int count = 0;
    foreach (const QString& fmt, formats) {
        if (count % 3 == 0) {
            rowLayout = new QHBoxLayout();
            layout->addLayout(rowLayout);
        }
        QCheckBox* cb = new QCheckBox(fmt.toUpper(), m_videoGroup);
        cb->setChecked(defaultFmts.contains(fmt));
        cb->setMinimumWidth(DPIAdapter::scaledSize(100));
        m_videoChecks.append(cb);
        rowLayout->addWidget(cb);
        count++;
    }
    qobject_cast<QVBoxLayout*>(m_videoGroup->layout())->addLayout(rowLayout);

    qobject_cast<QVBoxLayout*>(parent->layout())->addWidget(m_videoGroup);
}

void FileAssocDialog::setupAudioSection(QWidget* parent)
{
    m_audioGroup = new QGroupBox("音频格式", parent);
    QVBoxLayout* layout = new QVBoxLayout(m_audioGroup);

    QStringList formats = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus", "aiff"};
    QStringList defaultFmts = {"mp3", "wav", "flac", "aac", "ogg", "m4a"};

    QHBoxLayout* rowLayout = nullptr;
    int count = 0;
    foreach (const QString& fmt, formats) {
        if (count % 3 == 0) {
            rowLayout = new QHBoxLayout();
            layout->addLayout(rowLayout);
        }
        QCheckBox* cb = new QCheckBox(fmt.toUpper(), m_audioGroup);
        cb->setChecked(defaultFmts.contains(fmt));
        cb->setMinimumWidth(DPIAdapter::scaledSize(100));
        m_audioChecks.append(cb);
        rowLayout->addWidget(cb);
        count++;
    }

    qobject_cast<QVBoxLayout*>(parent->layout())->addWidget(m_audioGroup);
}

void FileAssocDialog::setupImageSection(QWidget* parent)
{
    m_imageGroup = new QGroupBox("图片格式", parent);
    QVBoxLayout* layout = new QVBoxLayout(m_imageGroup);

    QStringList formats = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "tiff", "svg", "ico"};
    QStringList defaultFmts = {"jpg", "jpeg", "png", "gif", "bmp", "webp"};

    QHBoxLayout* rowLayout = nullptr;
    int count = 0;
    foreach (const QString& fmt, formats) {
        if (count % 3 == 0) {
            rowLayout = new QHBoxLayout();
            layout->addLayout(rowLayout);
        }
        QCheckBox* cb = new QCheckBox(fmt.toUpper(), m_imageGroup);
        cb->setChecked(defaultFmts.contains(fmt));
        cb->setMinimumWidth(DPIAdapter::scaledSize(100));
        m_imageChecks.append(cb);
        rowLayout->addWidget(cb);
        count++;
    }

    qobject_cast<QVBoxLayout*>(parent->layout())->addWidget(m_imageGroup);
}

void FileAssocDialog::saveAssociations()
{
    FileAssociation* fa = FileAssociation::instance();

    QStringList videos;
    foreach (QCheckBox* cb, m_videoChecks) {
        if (cb->isChecked()) {
            videos.append(cb->text().toLower());
        }
    }
    fa->setVideoFormats(videos);

    QStringList audios;
    foreach (QCheckBox* cb, m_audioChecks) {
        if (cb->isChecked()) {
            audios.append(cb->text().toLower());
        }
    }
    fa->setAudioFormats(audios);

    QStringList images;
    foreach (QCheckBox* cb, m_imageChecks) {
        if (cb->isChecked()) {
            images.append(cb->text().toLower());
        }
    }
    fa->setImageFormats(images);
}
