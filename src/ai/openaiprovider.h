#ifndef OPENAIPROVIDER_H
#define OPENAIPROVIDER_H

#include "aiprovider.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class OpenAIProvider : public AIProvider
{
    Q_OBJECT

public:
    explicit OpenAIProvider(QObject *parent = nullptr);
    ~OpenAIProvider();

    QString getName() const override;
    bool isConfigured() const override;
    void configure(const QMap<QString, QString> &config) override;
    QMap<QString, QString> getConfiguration() const override;

public slots:
    void sendRequest(const AIRequest &request) override;

private slots:
    void onNetworkReply(QNetworkReply *reply);
    void onRequestTimeout();

private:
    QString buildPrompt(const AIRequest &request);
    void setupRequestHeaders(QNetworkRequest &request);
    QByteArray buildRequestBody(const QString &prompt);

    QNetworkAccessManager *m_networkManager;
    QMap<QString, QString> m_config;
    QTimer m_requestTimer;
    QList<QNetworkReply*> m_pendingReplies;
    bool m_isConfigured;
};

#endif // OPENAIPROVIDER_H
