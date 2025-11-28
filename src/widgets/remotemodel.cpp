#include "remotemodel.h"

RemoteModel::RemoteModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

RemoteModel::~RemoteModel()
{
}

void RemoteModel::setRemotes(const QList<GitManager::RemoteInfo> &remotes)
{
    beginResetModel();
    m_remotes = remotes;
    endResetModel();
}

GitManager::RemoteInfo RemoteModel::getRemoteInfo(int row) const
{
    if (row >= 0 && row < m_remotes.size()) {
        return m_remotes[row];
    }
    return GitManager::RemoteInfo();
}

int RemoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_remotes.size();
}

QVariant RemoteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_remotes.size()) {
        return QVariant();
    }

    const GitManager::RemoteInfo &remoteInfo = m_remotes[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return remoteInfo.name + " - " + remoteInfo.url;
        break;
    case Qt::ToolTipRole:
        return "名称: " + remoteInfo.name + "\nURL: " + remoteInfo.url;
        break;
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant RemoteModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "远程仓库";
    }
    return QVariant();
}
