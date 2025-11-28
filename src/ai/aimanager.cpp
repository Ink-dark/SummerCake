#include "aimanager.h"
#include "openaiprovider.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

AIManager::AIManager(QObject *parent)
    : QObject(parent),
      m_currentProvider(nullptr),
      m_aiEnabled(true),
      m_privacyModeEnabled(false)
{
    // 初始化配置文件路径
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir appDir(appDataPath);
    if (!appDir.exists()) {
        appDir.mkpath(".");
    }
    m_configFilePath = appDir.filePath("ai_config.ini");
    
    // 注册默认提供商
    registerProvider(new OpenAIProvider(this));
    
    // 加载配置
    loadConfiguration();
}

AIManager::~AIManager()
{
    // 保存配置
    saveConfiguration();
    
    // 清理所有提供商
    for (AIProvider *provider : m_providers.values()) {
        delete provider;
    }
    m_providers.clear();
    m_currentProvider = nullptr;
}

void AIManager::registerProvider(AIProvider *provider)
{
    if (!provider) return;
    
    QString providerName = provider->getName();
    if (m_providers.contains(providerName)) {
        delete m_providers[providerName];
    }
    
    m_providers[providerName] = provider;
    
    // 连接信号槽
    connect(provider, &AIProvider::responseReady, this, &AIManager::onProviderResponse);
    connect(provider, &AIProvider::errorOccurred, this, &AIManager::onProviderError);
    connect(provider, &AIProvider::statusChanged, this, &AIManager::onProviderStatusChanged);
    
    // 如果还没有当前提供商，设置为第一个注册的提供商
    if (!m_currentProvider) {
        setCurrentProvider(providerName);
    }
}

void AIManager::unregisterProvider(const QString &providerName)
{
    if (m_providers.contains(providerName)) {
        AIProvider *provider = m_providers[providerName];
        
        // 如果是当前提供商，清除当前提供商
        if (m_currentProvider == provider) {
            m_currentProvider = nullptr;
        }
        
        // 断开信号槽
        disconnect(provider, &AIProvider::responseReady, this, &AIManager::onProviderResponse);
        disconnect(provider, &AIProvider::errorOccurred, this, &AIManager::onProviderError);
        disconnect(provider, &AIProvider::statusChanged, this, &AIManager::onProviderStatusChanged);
        
        // 删除提供商
        delete provider;
        m_providers.remove(providerName);
    }
}

QList<QString> AIManager::getAvailableProviders() const
{
    return m_providers.keys();
}

AIProvider *AIManager::getProvider(const QString &providerName) const
{
    return m_providers.value(providerName, nullptr);
}

AIProvider *AIManager::getCurrentProvider() const
{
    return m_currentProvider;
}

bool AIManager::setCurrentProvider(const QString &providerName)
{
    if (!m_providers.contains(providerName)) {
        return false;
    }
    
    m_currentProvider = m_providers[providerName];
    emit currentProviderChanged(providerName);
    return true;
}

bool AIManager::isAIEnabled() const
{
    return m_aiEnabled;
}

void AIManager::setAIEnabled(bool enabled)
{
    if (m_aiEnabled != enabled) {
        m_aiEnabled = enabled;
        emit aiEnabledChanged(enabled);
    }
}

bool AIManager::isPrivacyModeEnabled() const
{
    return m_privacyModeEnabled;
}

void AIManager::setPrivacyModeEnabled(bool enabled)
{
    if (m_privacyModeEnabled != enabled) {
        m_privacyModeEnabled = enabled;
        emit privacyModeChanged(enabled);
    }
}

void AIManager::configureProvider(const QString &providerName, const QMap<QString, QString> &config)
{
    AIProvider *provider = getProvider(providerName);
    if (provider) {
        provider->configure(config);
        saveConfiguration();
    }
}

QMap<QString, QString> AIManager::getProviderConfiguration(const QString &providerName) const
{
    AIProvider *provider = getProvider(providerName);
    if (provider) {
        return provider->getConfiguration();
    }
    return QMap<QString, QString>();
}

void AIManager::sendRequest(const AIProvider::AIRequest &request)
{
    if (!m_aiEnabled) {
        emit errorOccurred("AI功能已禁用");
        return;
    }
    
    if (m_privacyModeEnabled) {
        // 隐私模式下，只允许本地模型
        if (m_currentProvider && m_currentProvider->getName() != "LocalAI") {
            emit errorOccurred("隐私模式下只允许使用本地模型");
            return;
        }
    }
    
    if (m_currentProvider) {
        m_currentProvider->sendRequest(request);
    } else {
        emit errorOccurred("未选择AI提供商");
    }
}

void AIManager::loadConfiguration()
{
    QSettings settings(m_configFilePath, QSettings::IniFormat);
    
    // 加载全局设置
    m_aiEnabled = settings.value("ai_enabled", true).toBool();
    m_privacyModeEnabled = settings.value("privacy_mode", false).toBool();
    QString currentProviderName = settings.value("current_provider", "OpenAI").toString();
    
    // 加载每个提供商的配置
    for (const QString &providerName : m_providers.keys()) {
        QSettings providerSettings(m_configFilePath, QSettings::IniFormat);
        providerSettings.beginGroup(providerName);
        
        QMap<QString, QString> config;
        for (const QString &key : providerSettings.allKeys()) {
            config[key] = providerSettings.value(key).toString();
        }
        
        m_providers[providerName]->configure(config);
        providerSettings.endGroup();
    }
    
    // 设置当前提供商
    setCurrentProvider(currentProviderName);
    
    emit aiEnabledChanged(m_aiEnabled);
    emit privacyModeChanged(m_privacyModeEnabled);
}

void AIManager::saveConfiguration()
{
    QSettings settings(m_configFilePath, QSettings::IniFormat);
    
    // 保存全局设置
    settings.setValue("ai_enabled", m_aiEnabled);
    settings.setValue("privacy_mode", m_privacyModeEnabled);
    if (m_currentProvider) {
        settings.setValue("current_provider", m_currentProvider->getName());
    }
    
    // 保存每个提供商的配置
    for (const QString &providerName : m_providers.keys()) {
        AIProvider *provider = m_providers[providerName];
        QMap<QString, QString> config = provider->getConfiguration();
        
        settings.beginGroup(providerName);
        for (const QString &key : config.keys()) {
            settings.setValue(key, config[key]);
        }
        settings.endGroup();
    }
    
    settings.sync();
}

void AIManager::onProviderResponse(const AIProvider::AIResponse &response)
{
    emit responseReady(response);
}

void AIManager::onProviderError(const QString &error)
{
    emit errorOccurred(error);
}

void AIManager::onProviderStatusChanged(bool isAvailable)
{
    AIProvider *senderProvider = qobject_cast<AIProvider*>(sender());
    if (senderProvider) {
        emit providerStatusChanged(senderProvider->getName(), isAvailable);
    }
}
