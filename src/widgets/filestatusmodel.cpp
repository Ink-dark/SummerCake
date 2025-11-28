#include "filestatusmodel.h"
#include <QIcon>
#include <QBrush>
#include <QColor>

FileStatusModel::FileStatusModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

FileStatusModel::~FileStatusModel()
{
}

void FileStatusModel::setFileStatus(const QList<GitManager::FileInfo> &fileStatus)
{
    beginResetModel();
    m_fileStatus = fileStatus;
    endResetModel();
}

GitManager::FileInfo FileStatusModel::getFileInfo(int row) const
{
    if (row >= 0 && row < m_fileStatus.size()) {
        return m_fileStatus[row];
    }
    return GitManager::FileInfo();
}

int FileStatusModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_fileStatus.size();
}

int FileStatusModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant FileStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_fileStatus.size() || index.column() >= ColumnCount) {
        return QVariant();
    }

    const GitManager::FileInfo &fileInfo = m_fileStatus[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case StatusColumn:
            return statusToString(fileInfo.status);
        case PathColumn:
            return fileInfo.path;
        case OldPathColumn:
            return fileInfo.oldPath;
        default:
            return QVariant();
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == StatusColumn) {
            return statusToIcon(fileInfo.status);
        }
        break;

    case Qt::ForegroundRole:
        switch (fileInfo.status) {
        case GitManager::Modified:
            return QBrush(QColor(255, 140, 0)); // 橙色
        case GitManager::Staged:
            return QBrush(QColor(0, 128, 0)); // 绿色
        case GitManager::Untracked:
            return QBrush(QColor(128, 128, 128)); // 灰色
        case GitManager::Deleted:
            return QBrush(QColor(255, 0, 0)); // 红色
        case GitManager::Renamed:
            return QBrush(QColor(0, 0, 255)); // 蓝色
        default:
            return QVariant();
        }
        break;

    default:
        return QVariant();
    }

    return QVariant();
}

QVariant FileStatusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case StatusColumn:
            return "状态";
        case PathColumn:
            return "路径";
        case OldPathColumn:
            return "原路径";
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QString FileStatusModel::statusToString(GitManager::FileStatus status) const
{
    switch (status) {
    case GitManager::Modified:
        return "已修改";
    case GitManager::Staged:
        return "已暂存";
    case GitManager::Untracked:
        return "未跟踪";
    case GitManager::Deleted:
        return "已删除";
    case GitManager::Renamed:
        return "已重命名";
    case GitManager::Ignored:
        return "已忽略";
    default:
        return "未知";
    }
}

QIcon FileStatusModel::statusToIcon(GitManager::FileStatus status) const
{
    // 这里可以返回实际的图标，暂时返回空图标
    // 后续可以添加资源文件和图标
    return QIcon();
}
