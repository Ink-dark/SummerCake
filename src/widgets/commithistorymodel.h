#ifndef COMMITHISTORYMODEL_H
#define COMMITHISTORYMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "git/gitmanager.h"

class CommitHistoryModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        HashColumn,
        AuthorColumn,
        DateColumn,
        MessageColumn,
        ColumnCount
    };

    explicit CommitHistoryModel(QObject *parent = nullptr);
    ~CommitHistoryModel();

    void setCommitHistory(const QList<GitManager::CommitInfo> &commitHistory);
    GitManager::CommitInfo getCommitInfo(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<GitManager::CommitInfo> m_commitHistory;
};

#endif // COMMITHISTORYMODEL_H
