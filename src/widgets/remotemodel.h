#ifndef REMOTEMODEL_H
#define REMOTEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "git/gitmanager.h"

class RemoteModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit RemoteModel(QObject *parent = nullptr);
    ~RemoteModel();

    void setRemotes(const QList<GitManager::RemoteInfo> &remotes);
    GitManager::RemoteInfo getRemoteInfo(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<GitManager::RemoteInfo> m_remotes;
};

#endif // REMOTEMODEL_H
