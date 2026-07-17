#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class HeaderBar : public QWidget
{
    Q_OBJECT

public:
    explicit HeaderBar(QWidget* parent = nullptr);
    ~HeaderBar();

    void setTitle(const QString& title);

signals:
    void openFileClicked();
    void minimizeClicked();
    void maximizeClicked();
    void closeClicked();

private:
    void setupUI();
    void updateTitleElided();

    QLabel* m_logoLabel;
    QLabel* m_titleLabel;
    QPushButton* m_minimizeBtn;
    QPushButton* m_maximizeBtn;
    QPushButton* m_closeBtn;
    QString m_title;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif
