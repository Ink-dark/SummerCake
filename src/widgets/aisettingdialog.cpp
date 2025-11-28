#include "aisettingdialog.h"
#include "ui_aisettingdialog.h"
#include <QMessageBox>
#include <QDebug>

AISettingDialog::AISettingDialog(AIManager *aiManager, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::AISettingDialog),
      m_aiManager(aiManager),
      m_currentProvider("")
{
    ui->setupUi(this);
    setupConnections();
    loadSettings();
}

AISettingDialog::~AISettingDialog()
{
    delete ui;
}

void AISettingDialog::setupConnections()
{
    // 连接信号槽
    connect(ui->providerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AISettingDialog::onProviderChanged);
    connect(ui->saveButton, &QPushButton::clicked, this, &AISettingDialog::onSaveSettings);
    connect(ui->testButton, &QPushButton::clicked, this, &AISettingDialog::onTestConnection);
    connect(ui->resetButton, &QPushButton::clicked, this, &AISettingDialog::onResetSettings);
    connect(ui->cancelButton, &QPushButton::clicked, this, &AISettingDialog::reject);
}

void AISettingDialog::loadSettings()
{
    // 加载通用设置
    ui->aiEnabledCheckBox->setChecked(m_aiManager->isAIEnabled());
    ui->privacyModeCheckBox->setChecked(m_aiManager->isPrivacyModeEnabled());
    
    // 加载AI服务提供商列表
    QList<QString> providers = m_aiManager->getAvailableProviders();
    ui->providerComboBox->addItems(providers);
    
    // 设置当前提供商
    AIProvider *currentProvider = m_aiManager->getCurrentProvider();
    if (currentProvider) {
        QString providerName = currentProvider->getName();
        int index = ui->providerComboBox->findText(providerName);
        if (index >= 0) {
            ui->providerComboBox->setCurrentIndex(index);
            m_currentProvider = providerName;
            updateProviderSettings();
        }
    }
}

void AISettingDialog::updateProviderSettings()
{
    // 清除当前提供商设置
    for (QWidget *widget : m_providerSettingsWidgets.values()) {
        delete widget;
    }
    m_providerSettingsWidgets.clear();
    
    // 获取当前提供商配置
    QMap<QString, QString> config = m_aiManager->getProviderConfiguration(m_currentProvider);
    
    // 根据提供商类型添加不同的设置项
    if (m_currentProvider == "OpenAI") {
        // OpenAI设置
        QLabel *apiKeyLabel = new QLabel("API密钥:", this);
        QLineEdit *apiKeyLineEdit = new QLineEdit(this);
        apiKeyLineEdit->setEchoMode(QLineEdit::Password);
        apiKeyLineEdit->setText(config.value("api_key", ""));
        m_providerSettingsWidgets["api_key"] = apiKeyLineEdit;
        ui->providerSettingsLayout->addRow(apiKeyLabel, apiKeyLineEdit);
        
        QLabel *modelLabel = new QLabel("模型:", this);
        QLineEdit *modelLineEdit = new QLineEdit(this);
        modelLineEdit->setText(config.value("model", "gpt-3.5-turbo"));
        m_providerSettingsWidgets["model"] = modelLineEdit;
        ui->providerSettingsLayout->addRow(modelLabel, modelLineEdit);
        
        QLabel *temperatureLabel = new QLabel("温度:", this);
        QLineEdit *temperatureLineEdit = new QLineEdit(this);
        temperatureLineEdit->setText(config.value("temperature", "0.7"));
        m_providerSettingsWidgets["temperature"] = temperatureLineEdit;
        ui->providerSettingsLayout->addRow(temperatureLabel, temperatureLineEdit);
        
        QLabel *maxTokensLabel = new QLabel("最大令牌数:", this);
        QLineEdit *maxTokensLineEdit = new QLineEdit(this);
        maxTokensLineEdit->setText(config.value("max_tokens", "500"));
        m_providerSettingsWidgets["max_tokens"] = maxTokensLineEdit;
        ui->providerSettingsLayout->addRow(maxTokensLabel, maxTokensLineEdit);
    }
    // 可以添加其他提供商的设置项
}

void AISettingDialog::onProviderChanged(int index)
{
    m_currentProvider = ui->providerComboBox->itemText(index);
    updateProviderSettings();
}

void AISettingDialog::onSaveSettings()
{
    // 保存通用设置
    m_aiManager->setAIEnabled(ui->aiEnabledCheckBox->isChecked());
    m_aiManager->setPrivacyModeEnabled(ui->privacyModeCheckBox->isChecked());
    
    // 保存当前提供商
    m_aiManager->setCurrentProvider(m_currentProvider);
    
    // 保存提供商配置
    QMap<QString, QString> config;
    for (const QString &key : m_providerSettingsWidgets.keys()) {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(m_providerSettingsWidgets[key]);
        if (lineEdit) {
            config[key] = lineEdit->text();
        }
    }
    m_aiManager->configureProvider(m_currentProvider, config);
    
    // 保存配置到文件
    m_aiManager->saveConfiguration();
    
    QMessageBox::information(this, "保存成功", "AI设置已保存");
    accept();
}

void AISettingDialog::onTestConnection()
{
    // 测试AI服务连接
    QMessageBox::information(this, "测试连接", "连接测试功能开发中");
}

void AISettingDialog::onResetSettings()
{
    // 重置设置
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "重置设置", "确定要重置所有AI设置吗？",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 重置通用设置
        ui->aiEnabledCheckBox->setChecked(true);
        ui->privacyModeCheckBox->setChecked(false);
        
        // 重置提供商设置
        updateProviderSettings();
    }
}
