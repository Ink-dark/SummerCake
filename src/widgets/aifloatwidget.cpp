#include "aifloatwidget.h"
#include <QMouseEvent>
#include <QStyle>
#include <QPalette>

AIFloatWidget::AIFloatWidget(QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
      m_isDragging(false)
{
    setupUI();
    setupConnections();
    setWindowTitle("AI助手");
    resize(300, 400);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(255, 255, 255, 0.95); border: 1px solid #ccc; border-radius: 8px;");
}

AIFloatWidget::~AIFloatWidget()
{
}

void AIFloatWidget::setupUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建响应显示区域
    m_responseTextEdit = new QTextEdit(this);
    m_responseTextEdit->setReadOnly(true);
    m_responseTextEdit->setPlaceholderText("AI助手响应将显示在这里...");
    m_responseTextEdit->setStyleSheet("background-color: white; border: 1px solid #ddd; border-radius: 4px;");
    m_mainLayout->addWidget(m_responseTextEdit, 1);
    
    // 创建输入布局
    m_inputLayout = new QHBoxLayout();
    m_inputLayout->setSpacing(5);
    
    // 创建查询输入框
    m_queryLineEdit = new QLineEdit(this);
    m_queryLineEdit->setPlaceholderText("输入您的问题...");
    m_queryLineEdit->setStyleSheet("background-color: white; border: 1px solid #ddd; border-radius: 4px;");
    m_inputLayout->addWidget(m_queryLineEdit, 1);
    
    // 创建发送按钮
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setStyleSheet("background-color: #4CAF50; color: white; border: none; border-radius: 4px; padding: 5px 10px;");
    m_inputLayout->addWidget(m_sendButton);
    
    // 创建关闭按钮
    m_closeButton = new QPushButton("关闭", this);
    m_closeButton->setStyleSheet("background-color: #f44336; color: white; border: none; border-radius: 4px; padding: 5px 10px;");
    m_inputLayout->addWidget(m_closeButton);
    
    // 将输入布局添加到主布局
    m_mainLayout->addLayout(m_inputLayout);
    
    setLayout(m_mainLayout);
}

void AIFloatWidget::setupConnections()
{
    connect(m_sendButton, &QPushButton::clicked, this, &AIFloatWidget::onSendQuery);
    connect(m_closeButton, &QPushButton::clicked, this, &AIFloatWidget::hideWidget);
    connect(m_queryLineEdit, &QLineEdit::returnPressed, this, &AIFloatWidget::onSendQuery);
}

void AIFloatWidget::showWidget()
{
    QWidget::show();
}

void AIFloatWidget::hideWidget()
{
    QWidget::hide();
}

bool AIFloatWidget::isVisible() const
{
    return QWidget::isVisible();
}

void AIFloatWidget::onSendQuery()
{
    QString query = m_queryLineEdit->text().trimmed();
    if (!query.isEmpty()) {
        // 在响应区域显示用户查询
        m_responseTextEdit->append("用户: " + query);
        m_responseTextEdit->append("AI: 思考中...");
        
        // 清空输入框
        m_queryLineEdit->clear();
        
        // 发送查询信号
        emit sendQuery(query);
    }
}

void AIFloatWidget::onResponseReceived(const QString &response)
{
    // 替换"思考中..."为实际响应
    QString currentText = m_responseTextEdit->toPlainText();
    if (currentText.endsWith("AI: 思考中...")) {
        currentText.chop(10); // 移除"AI: 思考中..."
        m_responseTextEdit->setPlainText(currentText);
    }
    
    m_responseTextEdit->append("AI: " + response);
    m_responseTextEdit->append(""); // 添加空行分隔
}

void AIFloatWidget::onErrorReceived(const QString &error)
{
    // 替换"思考中..."为错误信息
    QString currentText = m_responseTextEdit->toPlainText();
    if (currentText.endsWith("AI: 思考中...")) {
        currentText.chop(10); // 移除"AI: 思考中..."
        m_responseTextEdit->setPlainText(currentText);
    }
    
    m_responseTextEdit->append("AI错误: " + error);
    m_responseTextEdit->append(""); // 添加空行分隔
}

void AIFloatWidget::closeEvent(QCloseEvent *event)
{
    hideWidget();
    emit widgetClosed();
    event->ignore(); // 忽略关闭事件，改为隐藏窗口
}

void AIFloatWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit widgetShown();
}

void AIFloatWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    emit widgetHidden();
}

void AIFloatWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void AIFloatWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && m_isDragging) {
        move(event->globalPosition().toPoint() - m_dragStartPosition);
        event->accept();
    }
}
