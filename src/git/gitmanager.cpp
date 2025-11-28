#include "gitmanager.h"
#include <QDir>
#include <QDebug>

GitManager::GitManager(QObject *parent)
    : QObject(parent), m_process(new QProcess(this))
{
    m_process->setProcessChannelMode(QProcess::MergedChannels);
}

GitManager::~GitManager()
{
    delete m_process;
}

bool GitManager::openRepository(const QString &path)
{
    QDir repoDir(path);
    if (!repoDir.exists(".git")) {
        emit errorOccurred("不是有效的Git仓库");
        return false;
    }

    m_currentRepository = path;
    emit repositoryOpened(path);
    return true;
}

bool GitManager::cloneRepository(const QString &url, const QString &destPath)
{
    QProcess cloneProcess;
    cloneProcess.setProcessChannelMode(QProcess::MergedChannels);
    
    QStringList args;
    args << "clone" << url << destPath;
    
    cloneProcess.start("git", args);
    if (!cloneProcess.waitForFinished(-1)) {
        emit errorOccurred(cloneProcess.readAllStandardOutput());
        return false;
    }
    
    if (cloneProcess.exitCode() != 0) {
        emit errorOccurred(cloneProcess.readAllStandardOutput());
        return false;
    }
    
    return openRepository(destPath);
}

bool GitManager::initRepository(const QString &path)
{
    QProcess initProcess;
    initProcess.setProcessChannelMode(QProcess::MergedChannels);
    initProcess.setWorkingDirectory(path);
    
    QStringList args;
    args << "init";
    
    initProcess.start("git", args);
    if (!initProcess.waitForFinished(-1)) {
        emit errorOccurred(initProcess.readAllStandardOutput());
        return false;
    }
    
    if (initProcess.exitCode() != 0) {
        emit errorOccurred(initProcess.readAllStandardOutput());
        return false;
    }
    
    return openRepository(path);
}

QString GitManager::getCurrentRepository() const
{
    return m_currentRepository;
}

QString GitManager::executeCommand(const QStringList &args, bool *success)
{
    if (m_currentRepository.isEmpty()) {
        if (success) *success = false;
        emit errorOccurred("未打开任何仓库");
        return "";
    }

    m_process->setWorkingDirectory(m_currentRepository);
    m_process->start("git", args);
    
    if (!m_process->waitForFinished(-1)) {
        if (success) *success = false;
        QString error = m_process->readAllStandardOutput();
        emit errorOccurred(error);
        return error;
    }
    
    if (m_process->exitCode() != 0) {
        if (success) *success = false;
        QString error = m_process->readAllStandardOutput();
        emit errorOccurred(error);
        return error;
    }
    
    if (success) *success = true;
    QString output = m_process->readAllStandardOutput();
    emit commandExecuted("git " + args.join(" "), output);
    return output;
}

GitManager::FileStatus GitManager::parseFileStatus(const QString &statusCode)
{
    if (statusCode == "M") return Modified;
    if (statusCode == "A") return Staged;
    if (statusCode == "??") return Untracked;
    if (statusCode == "D") return Deleted;
    if (statusCode == "R") return Renamed;
    if (statusCode == "!!") return Ignored;
    return Unknown;
}

QList<GitManager::FileInfo> GitManager::getFileStatus()
{
    QList<FileInfo> fileList;
    QStringList args;
    args << "status" << "--porcelain" << "--ignore-submodules";
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return fileList;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.length() < 3) continue;
        
        FileInfo file;
        QString statusCode = line.left(2).trimmed();
        
        if (statusCode.startsWith("R")) {
            // 重命名的文件格式: R  oldpath -> newpath
            QStringList parts = line.mid(3).split(" -> ", Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                file.oldPath = parts[0].trimmed();
                file.path = parts[1].trimmed();
                file.status = Renamed;
            }
        } else {
            file.path = line.mid(3).trimmed();
            file.status = parseFileStatus(statusCode);
        }
        
        fileList.append(file);
    }
    
    return fileList;
}

bool GitManager::stageFile(const QString &filePath)
{
    QStringList args;
    args << "add" << filePath;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::unstageFile(const QString &filePath)
{
    QStringList args;
    args << "reset" << "HEAD" << filePath;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::commit(const QString &message)
{
    QStringList args;
    args << "commit" << "-m" << message;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::discardChanges(const QString &filePath)
{
    QStringList args;
    args << "checkout" << "--" << filePath;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

QList<GitManager::CommitInfo> GitManager::getCommitHistory(int limit)
{
    QList<CommitInfo> commitList;
    QStringList args;
    args << "log" << QString("--pretty=format:%H|%an|%ad|%s") << "--date=short" << QString("-n%1").arg(limit);
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return commitList;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QStringList parts = line.split("|");
        if (parts.size() < 4) continue;
        
        CommitInfo commit;
        commit.hash = parts[0];
        commit.author = parts[1];
        commit.date = parts[2];
        commit.message = parts[3];
        
        commitList.append(commit);
    }
    
    return commitList;
}

QString GitManager::getCommitDiff(const QString &commitHash)
{
    QStringList args;
    args << "show" << commitHash;
    
    bool success;
    QString output = executeCommand(args, &success);
    return success ? output : "";
}

GitManager::CommitInfo GitManager::getCommitInfo(const QString &commitHash)
{
    CommitInfo commit;
    QStringList args;
    args << "show" << commitHash << "--pretty=format:%H|%an|%ad|%s" << "--date=short" << "-s";
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return commit;
    
    QStringList parts = output.split("|");
    if (parts.size() >= 4) {
        commit.hash = parts[0];
        commit.author = parts[1];
        commit.date = parts[2];
        commit.message = parts[3];
    }
    
    return commit;
}

QList<GitManager::BranchInfo> GitManager::getBranches()
{
    QList<BranchInfo> branchList;
    QStringList args;
    args << "branch" << "-a";
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return branchList;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        BranchInfo branch;
        QString trimmedLine = line.trimmed();
        
        if (trimmedLine.startsWith("*")) {
            branch.isCurrent = true;
            branch.name = trimmedLine.mid(2);
        } else {
            branch.isCurrent = false;
            branch.name = trimmedLine;
        }
        
        branch.isRemote = branch.name.startsWith("remotes/");
        if (branch.isRemote) {
            // 移除remotes/前缀
            branch.name = branch.name.mid(8);
        }
        
        branchList.append(branch);
    }
    
    return branchList;
}

bool GitManager::createBranch(const QString &branchName)
{
    QStringList args;
    args << "branch" << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::checkoutBranch(const QString &branchName)
{
    QStringList args;
    args << "checkout" << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::mergeBranch(const QString &branchName)
{
    QStringList args;
    args << "merge" << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::deleteBranch(const QString &branchName)
{
    QStringList args;
    args << "branch" << "-d" << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

QList<GitManager::RemoteInfo> GitManager::getRemotes()
{
    QList<RemoteInfo> remoteList;
    QStringList args;
    args << "remote" << "-v";
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return remoteList;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QStringList parts = line.split("\t", Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;
        
        QString name = parts[0];
        QString url = parts[1].split(" ")[0];
        
        // 避免重复添加
        bool exists = false;
        for (const RemoteInfo &remote : remoteList) {
            if (remote.name == name) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            RemoteInfo remote;
            remote.name = name;
            remote.url = url;
            remoteList.append(remote);
        }
    }
    
    return remoteList;
}

bool GitManager::addRemote(const QString &name, const QString &url)
{
    QStringList args;
    args << "remote" << "add" << name << url;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::removeRemote(const QString &name)
{
    QStringList args;
    args << "remote" << "remove" << name;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::push(const QString &remoteName, const QString &branchName)
{
    QStringList args;
    args << "push" << remoteName << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::pull(const QString &remoteName, const QString &branchName)
{
    QStringList args;
    args << "pull" << remoteName << branchName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}

QString GitManager::getDiff(const QString &filePath)
{
    QStringList args;
    args << "diff" << filePath;
    
    bool success;
    QString output = executeCommand(args, &success);
    return success ? output : "";
}

QString GitManager::getStagedDiff(const QString &filePath)
{
    QStringList args;
    args << "diff" << "--staged" << filePath;
    
    bool success;
    QString output = executeCommand(args, &success);
    return success ? output : "";
}

QList<QString> GitManager::getTags()
{
    QList<QString> tagList;
    QStringList args;
    args << "tag";
    
    bool success;
    QString output = executeCommand(args, &success);
    if (!success) return tagList;
    
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        tagList.append(line.trimmed());
    }
    
    return tagList;
}

bool GitManager::createTag(const QString &tagName, const QString &commitHash)
{
    QStringList args;
    args << "tag" << tagName;
    if (!commitHash.isEmpty()) {
        args << commitHash;
    }
    
    bool success;
    executeCommand(args, &success);
    return success;
}

bool GitManager::deleteTag(const QString &tagName)
{
    QStringList args;
    args << "tag" << "-d" << tagName;
    
    bool success;
    executeCommand(args, &success);
    return success;
}
