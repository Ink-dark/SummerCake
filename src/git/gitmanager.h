#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <QList>
#include <QMap>
#include <QPair>

class GitManager : public QObject
{
    Q_OBJECT

public:
    enum FileStatus {
        Modified,
        Staged,
        Untracked,
        Deleted,
        Renamed,
        Ignored,
        Unknown
    };

    struct FileInfo {
        QString path;
        FileStatus status;
        QString oldPath; // For renamed files
    };

    struct CommitInfo {
        QString hash;
        QString author;
        QString date;
        QString message;
    };

    struct BranchInfo {
        QString name;
        bool isCurrent;
        bool isRemote;
    };

    struct RemoteInfo {
        QString name;
        QString url;
    };

    explicit GitManager(QObject *parent = nullptr);
    ~GitManager();

    // 仓库操作
    bool openRepository(const QString &path);
    bool cloneRepository(const QString &url, const QString &destPath);
    bool initRepository(const QString &path);
    QString getCurrentRepository() const;

    // 文件状态操作
    QList<FileInfo> getFileStatus();
    bool stageFile(const QString &filePath);
    bool unstageFile(const QString &filePath);
    bool commit(const QString &message);
    bool discardChanges(const QString &filePath);

    // 提交历史
    QList<CommitInfo> getCommitHistory(int limit = 100);
    QString getCommitDiff(const QString &commitHash);
    CommitInfo getCommitInfo(const QString &commitHash);

    // 分支操作
    QList<BranchInfo> getBranches();
    bool createBranch(const QString &branchName);
    bool checkoutBranch(const QString &branchName);
    bool mergeBranch(const QString &branchName);
    bool deleteBranch(const QString &branchName);

    // 远程操作
    QList<RemoteInfo> getRemotes();
    bool addRemote(const QString &name, const QString &url);
    bool removeRemote(const QString &name);
    bool push(const QString &remoteName, const QString &branchName);
    bool pull(const QString &remoteName, const QString &branchName);

    // 差异对比
    QString getDiff(const QString &filePath);
    QString getStagedDiff(const QString &filePath);

    // 标签操作
    QList<QString> getTags();
    bool createTag(const QString &tagName, const QString &commitHash = "");
    bool deleteTag(const QString &tagName);

signals:
    void repositoryOpened(const QString &path);
    void repositoryClosed();
    void commandExecuted(const QString &command, const QString &output);
    void errorOccurred(const QString &error);

private:
    QString executeCommand(const QStringList &args, bool *success = nullptr);
    FileStatus parseFileStatus(const QString &statusCode);

    QString m_currentRepository;
    QProcess *m_process;
};

#endif // GITMANAGER_H
