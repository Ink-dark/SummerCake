#ifndef AIPROVIDER_H
#define AIPROVIDER_H

#include <QObject>
#include <QString>
#include <QMap>

class AIProvider : public QObject
{
    Q_OBJECT

public:
    enum AIRequestType {
        GenerateCommitMessage,
        CodeReview,
        ResolveConflict,
        ExplainGitCommand,
        RepositoryHealth,
        SmartCompletion
    };

    struct AIRequest {
        AIRequestType type;
        QString content;
        QMap<QString, QString> parameters;
    };

    struct AIResponse {
        bool success;
        QString content;
        QString errorMessage;
    };

    explicit AIProvider(QObject *parent = nullptr);
    virtual ~AIProvider();

    virtual QString getName() const = 0;
    virtual bool isConfigured() const = 0;
    virtual void configure(const QMap<QString, QString> &config) = 0;
    virtual QMap<QString, QString> getConfiguration() const = 0;

public slots:
    virtual void sendRequest(const AIRequest &request) = 0;

signals:
    void responseReady(const AIResponse &response);
    void errorOccurred(const QString &error);
    void statusChanged(bool isAvailable);
};

#endif // AIPROVIDER_H
