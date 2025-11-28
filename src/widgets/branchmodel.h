#ifndef BRANCHMODEL_H
#define BRANCHMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "git/gitmanager.h"

class BranchModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit BranchModel(QObject *parent = nullptr);
    ~BranchModel();

    void setBranches(const QList<GitManager::BranchInfo> &branches);
    GitManager::BranchInfo getBranchInfo(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QList<GitManager::BranchInfo> m_branches;
};

#endif // BRANCHMODEL_H
