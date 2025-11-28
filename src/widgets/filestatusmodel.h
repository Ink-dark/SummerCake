#ifndef FILESTATUSMODEL_H
#define FILESTATUSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "git/gitmanager.h"

class FileStatusModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        StatusColumn,
        PathColumn,
        OldPathColumn,
        ColumnCount
    };

    explicit FileStatusModel(QObject *parent = nullptr);
    ~FileStatusModel();

    void setFileStatus(const QList<GitManager::FileInfo> &fileStatus);
    GitManager::FileInfo getFileInfo(int row) const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    QString statusToString(GitManager::FileStatus status) const;
    QIcon statusToIcon(GitManager::FileStatus status) const;

    QList<GitManager::FileInfo> m_fileStatus;
};

#endif // FILESTATUSMODEL_H
