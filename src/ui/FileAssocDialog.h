#ifndef FILEASSOCDIALOG_H
#define FILEASSOCDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QGroupBox>

class FileAssocDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileAssocDialog(QWidget* parent = nullptr);
    ~FileAssocDialog();

    void saveAssociations();

private:
    void setupUI();
    void setupVideoSection(QWidget* parent);
    void setupAudioSection(QWidget* parent);
    void setupImageSection(QWidget* parent);

    QGroupBox* m_videoGroup;
    QGroupBox* m_audioGroup;
    QGroupBox* m_imageGroup;
    QList<QCheckBox*> m_videoChecks;
    QList<QCheckBox*> m_audioChecks;
    QList<QCheckBox*> m_imageChecks;
};

#endif
