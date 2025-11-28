#ifndef AIFLOATWIDGET_H
#define AIFLOATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QShowEvent>
#include <QHideEvent>

class AIFloatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIFloatWidget(QWidget *parent = nullptr);
    ~AIFloatWidget();

    void showWidget();
    void hideWidget();
    bool isVisible() const;

signals:
    void sendQuery(const QString &query);
    void widgetClosed();
    void widgetShown();
    void widgetHidden();

public slots:
    void onSendQuery();
    void onResponseReceived(const QString &response);
    void onErrorReceived(const QString &error);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void setupUI();
    void setupConnections();

    QTextEdit *m_responseTextEdit;
    QLineEdit *m_queryLineEdit;
    QPushButton *m_sendButton;
    QPushButton *m_closeButton;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_inputLayout;
    
    bool m_isDragging;
    QPoint m_dragStartPosition;
};

#endif // AIFLOATWIDGET_H
