#include "openaiprovider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>

OpenAIProvider::OpenAIProvider(QObject *parent)
    : AIProvider(parent),
      m_networkManager(new QNetworkAccessManager(this)),
      m_isConfigured(false)
{
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &OpenAIProvider::onNetworkReply);
    
    m_requestTimer.setSingleShot(true);
    m_requestTimer.setInterval(30000); // 30秒超时
    connect(&m_requestTimer, &QTimer::timeout, this, &OpenAIProvider::onRequestTimeout);
}

OpenAIProvider::~OpenAIProvider()
{
    // 取消所有待处理的请求
    for (QNetworkReply *reply : m_pendingReplies) {
        reply->abort();
        reply->deleteLater();
    }
    m_pendingReplies.clear();
    
    delete m_networkManager;
}

QString OpenAIProvider::getName() const
{
    return "OpenAI";
}

bool OpenAIProvider::isConfigured() const
{
    return m_isConfigured;
}

void OpenAIProvider::configure(const QMap<QString, QString> &config)
{
    m_config = config;
    m_isConfigured = m_config.contains("api_key") && !m_config["api_key"].isEmpty();
    
    // 设置默认值
    if (!m_config.contains("model")) {
        m_config["model"] = "gpt-3.5-turbo";
    }
    if (!m_config.contains("temperature")) {
        m_config["temperature"] = "0.7";
    }
    if (!m_config.contains("max_tokens")) {
        m_config["max_tokens"] = "500";
    }
    
    emit statusChanged(m_isConfigured);
}

QMap<QString, QString> OpenAIProvider::getConfiguration() const
{
    return m_config;
}

void OpenAIProvider::sendRequest(const AIRequest &request)
{
    if (!m_isConfigured) {
        emit errorOccurred("OpenAI未配置");
        return;
    }
    
    QString prompt = buildPrompt(request);
    QByteArray requestBody = buildRequestBody(prompt);
    
    QNetworkRequest networkRequest;
    networkRequest.setUrl(QUrl("https://api.openai.com/v1/chat/completions"));
    setupRequestHeaders(networkRequest);
    
    QNetworkReply *reply = m_networkManager->post(networkRequest, requestBody);
    m_pendingReplies.append(reply);
    
    // 启动超时计时器
    if (!m_requestTimer.isActive()) {
        m_requestTimer.start();
    }
}

void OpenAIProvider::onNetworkReply(QNetworkReply *reply)
{
    m_pendingReplies.removeOne(reply);
    
    // 如果没有更多待处理请求，停止计时器
    if (m_pendingReplies.isEmpty() && m_requestTimer.isActive()) {
        m_requestTimer.stop();
    }
    
    AIResponse response;
    response.success = false;
    
    if (reply->error() != QNetworkReply::NoError) {
        response.errorMessage = reply->errorString();
        emit errorOccurred(response.errorMessage);
    } else {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        
        if (jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            
            if (jsonObj.contains("error")) {
                QJsonObject errorObj = jsonObj["error"].toObject();
                response.errorMessage = errorObj["message"].toString();
                emit errorOccurred(response.errorMessage);
            } else if (jsonObj.contains("choices")) {
                QJsonArray choicesArray = jsonObj["choices"].toArray();
                if (!choicesArray.isEmpty()) {
                    QJsonObject choiceObj = choicesArray[0].toObject();
                    QJsonObject messageObj = choiceObj["message"].toObject();
                    response.content = messageObj["content"].toString().trimmed();
                    response.success = true;
                }
            }
        }
    }
    
    emit responseReady(response);
    reply->deleteLater();
}

void OpenAIProvider::onRequestTimeout()
{
    // 超时处理
    for (QNetworkReply *reply : m_pendingReplies) {
        reply->abort();
        reply->deleteLater();
    }
    m_pendingReplies.clear();
    
    emit errorOccurred("请求超时");
    
    AIResponse response;
    response.success = false;
    response.errorMessage = "请求超时";
    emit responseReady(response);
}

QString OpenAIProvider::buildPrompt(const AIRequest &request)
{
    QString prompt;
    
    switch (request.type) {
    case GenerateCommitMessage:
        prompt = "请根据以下Git修改内容生成一个简洁明了的提交信息：\n" + request.content;
        break;
        
    case CodeReview:
        prompt = "请对以下代码修改进行审查，提供改进建议：\n" + request.content;
        break;
        
    case ResolveConflict:
        prompt = "请帮助解决以下Git冲突：\n" + request.content;
        break;
        
    case ExplainGitCommand:
        prompt = "请解释以下Git命令的作用和用法：\n" + request.content;
        break;
        
    case RepositoryHealth:
        prompt = "请分析以下Git仓库信息，提供健康状况和优化建议：\n" + request.content;
        break;
        
    case SmartCompletion:
        prompt = "请根据上下文提供智能补全建议：\n" + request.content;
        break;
        
    default:
        prompt = request.content;
        break;
    }
    
    return prompt;
}

void OpenAIProvider::setupRequestHeaders(QNetworkRequest &request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString authHeader = "Bearer " + m_config["api_key"];
    request.setRawHeader("Authorization", authHeader.toUtf8());
}

QByteArray OpenAIProvider::buildRequestBody(const QString &prompt)
{
    QJsonObject requestBody;
    requestBody["model"] = m_config["model"];
    requestBody["temperature"] = m_config["temperature"].toDouble();
    requestBody["max_tokens"] = m_config["max_tokens"].toInt();
    
    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个Git助手，帮助用户管理Git仓库和编写高质量的提交信息。请使用中文回答。";
    messages.append(systemMessage);
    
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt;
    messages.append(userMessage);
    
    requestBody["messages"] = messages;
    
    QJsonDocument jsonDoc(requestBody);
    return jsonDoc.toJson();
}
