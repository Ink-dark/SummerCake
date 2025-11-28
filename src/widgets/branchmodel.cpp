#include "branchmodel.h"
#include <QBrush>
#include <QColor>

BranchModel::BranchModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

BranchModel::~BranchModel()
{
}

void BranchModel::setBranches(const QList<GitManager::BranchInfo> &branches)
{
    beginResetModel();
    m_branches = branches;
    endResetModel();
}

GitManager::BranchInfo BranchModel::getBranchInfo(int row) const
{
    if (row >= 0 && row < m_branches.size()) {
        return m_branches[row];
    }
    return GitManager::BranchInfo();
}

int BranchModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_branches.size();
}

QVariant BranchModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_branches.size()) {
        return QVariant();
    }

    const GitManager::BranchInfo &branchInfo = m_branches[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return branchInfo.name + (branchInfo.isCurrent ? " (当前)" : "");
        break;

    case Qt::ForegroundRole:
        if (branchInfo.isCurrent) {
            return QBrush(QColor(0, 128, 0)); // 当前分支显示为绿色
        } else if (branchInfo.isRemote) {
            return QBrush(QColor(0, 0, 255)); // 远程分支显示为蓝色
        }
        return QBrush(QColor(0, 0, 0)); // 其他分支显示为黑色
        break;

    case Qt::FontRole:
        if (branchInfo.isCurrent) {
            QFont font;
            font.setBold(true);
            return font;
        }
        break;

    default:
        return QVariant();
    }

    return QVariant();
}

QVariant BranchModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "分支";
    }
    return QVariant();
}
