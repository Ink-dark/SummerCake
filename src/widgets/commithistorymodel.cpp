#include "commithistorymodel.h"

CommitHistoryModel::CommitHistoryModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

CommitHistoryModel::~CommitHistoryModel()
{
}

void CommitHistoryModel::setCommitHistory(const QList<GitManager::CommitInfo> &commitHistory)
{
    beginResetModel();
    m_commitHistory = commitHistory;
    endResetModel();
}

GitManager::CommitInfo CommitHistoryModel::getCommitInfo(int row) const
{
    if (row >= 0 && row < m_commitHistory.size()) {
        return m_commitHistory[row];
    }
    return GitManager::CommitInfo();
}

int CommitHistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_commitHistory.size();
}

int CommitHistoryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant CommitHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_commitHistory.size() || index.column() >= ColumnCount) {
        return QVariant();
    }

    const GitManager::CommitInfo &commitInfo = m_commitHistory[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case HashColumn:
            return commitInfo.hash.left(7); // 只显示前7个字符
        case AuthorColumn:
            return commitInfo.author;
        case DateColumn:
            return commitInfo.date;
        case MessageColumn:
            return commitInfo.message;
        default:
            return QVariant();
        }
        break;

    case Qt::ToolTipRole:
        if (index.column() == HashColumn) {
            return commitInfo.hash; // 完整哈希值作为提示
        }
        break;

    default:
        return QVariant();
    }

    return QVariant();
}

QVariant CommitHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case HashColumn:
            return "哈希值";
        case AuthorColumn:
            return "作者";
        case DateColumn:
            return "日期";
        case MessageColumn:
            return "提交信息";
        default:
            return QVariant();
        }
    }
    return QVariant();
}
