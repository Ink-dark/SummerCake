#ifndef AIMANAGER_H
#define AIMANAGER_H

#include "aiprovider.h"
#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

class AIManager : public QObject
{
    Q_OBJECT

public:
    explicit AIManager(QObject *parent = nullptr);
    ~AIManager();

    void registerProvider(AIProvider *provider);
    void unregisterProvider(const QString &providerName);
    QList<QString> getAvailableProviders() const;
    AIProvider *getProvider(const QString &providerName) const;
    AIProvider *getCurrentProvider() const;
    bool setCurrentProvider(const QString &providerName);

    bool isAIEnabled() const;
    void setAIEnabled(bool enabled);

    bool isPrivacyModeEnabled() const;
    void setPrivacyModeEnabled(bool enabled);

    void configureProvider(const QString &providerName, const QMap<QString, QString> &config);
    QMap<QString, QString> getProviderConfiguration(const QString &providerName) const;

public slots:
    void sendRequest(const AIProvider::AIRequest &request);
    void loadConfiguration();
    void saveConfiguration();

signals:
    void responseReady(const AIProvider::AIResponse &response);
    void errorOccurred(const QString &error);
    void providerStatusChanged(const QString &providerName, bool isAvailable);
    void currentProviderChanged(const QString &providerName);
    void aiEnabledChanged(bool enabled);
    void privacyModeChanged(bool enabled);

private slots:
    void onProviderResponse(const AIProvider::AIResponse &response);
    void onProviderError(const QString &error);
    void onProviderStatusChanged(bool isAvailable);

private:
    QMap<QString, AIProvider*> m_providers;
    AIProvider *m_currentProvider;
    bool m_aiEnabled;
    bool m_privacyModeEnabled;
    QString m_configFilePath;
};

#endif // AIMANAGER_H
