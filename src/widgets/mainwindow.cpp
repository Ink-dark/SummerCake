#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filestatusmodel.h"
#include "commithistorymodel.h"
#include "branchmodel.h"
#include "remotemodel.h"
#include "aifloatwidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_gitManager(new GitManager(this)),
      m_aiManager(new AIManager(this)),
      m_aiEnabled(true),
      m_privacyModeEnabled(false),
      m_currentAIRequestType(AIProvider::GenerateCommitMessage),
      m_aiCommitSuggestion(""),
      m_aiFloatWidget(nullptr)
{
    ui->setupUi(this);
    setupUI();
    setupMenus();
    setupToolBar();
    setupConnections();
    updateStatusBar();
    
    // 初始化AI悬浮窗
    m_aiFloatWidget = new AIFloatWidget(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 设置窗口大小和标题
    setWindowTitle("SummerCake - Git GUI");
    resize(1200, 800);
    
    // 初始化文件系统模型
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setRootPath("");
    ui->repoTreeView->setModel(m_fileSystemModel);
    
    // 初始化文件状态模型
    m_fileStatusModel = new FileStatusModel(this);
    ui->fileStatusView->setModel(m_fileStatusModel);
    
    // 设置文件状态表格视图属性
    ui->fileStatusView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->fileStatusView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->fileStatusView->setAlternatingRowColors(true);
    ui->fileStatusView->resizeColumnsToContents();
    
    // 连接文件状态右键菜单
    ui->fileStatusView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->fileStatusView, &QTableView::customContextMenuRequested, this, &MainWindow::onFileStatusContextMenu);
    
    // 初始化提交历史模型
    m_commitHistoryModel = new CommitHistoryModel(this);
    ui->commitHistoryView->setModel(m_commitHistoryModel);
    
    // 设置提交历史表格视图属性
    ui->commitHistoryView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->commitHistoryView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->commitHistoryView->setAlternatingRowColors(true);
    ui->commitHistoryView->resizeColumnsToContents();
    ui->commitHistoryView->setColumnWidth(HashColumn, 100);
    ui->commitHistoryView->setColumnWidth(AuthorColumn, 150);
    ui->commitHistoryView->setColumnWidth(DateColumn, 120);
    ui->commitHistoryView->horizontalHeader()->setStretchLastSection(true);
    
    // 初始化分支模型
    m_branchModel = new BranchModel(this);
    ui->branchListView->setModel(m_branchModel);
    
    // 设置分支列表视图属性
    ui->branchListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->branchListView->setAlternatingRowColors(true);
    
    // 设置分割器比例
    ui->mainSplitter->setSizes(QList<int>({250, 500, 250}));
    
    // 初始化状态栏
    ui->branchLabel->setText("分支: 未打开仓库");
    ui->repoStatusLabel->setText("状态: 未打开仓库");
    ui->aiStatusLabel->setText("AI: " + (m_aiEnabled ? "已启用" : "已禁用"));
    
    // 禁用初始状态下不可用的功能
    ui->actionCommit->setEnabled(false);
    ui->actionPush->setEnabled(false);
    ui->actionPull->setEnabled(false);
}

void MainWindow::setupMenus()
{
    // 文件菜单
    m_actionOpenRepo = new QAction("打开仓库", this);
    m_actionCloneRepo = new QAction("克隆仓库", this);
    m_actionInitRepo = new QAction("初始化仓库", this);
    m_actionSettings = new QAction("设置", this);
    
    ui->menuFile->addAction(m_actionOpenRepo);
    ui->menuFile->addAction(m_actionCloneRepo);
    ui->menuFile->addAction(m_actionInitRepo);
    ui->menuFile->addSeparator();
    ui->menuFile->addAction(m_actionSettings);
    
    // 仓库菜单
    m_actionCommit = new QAction("提交", this);
    m_actionPush = new QAction("推送", this);
    m_actionPull = new QAction("拉取", this);
    
    ui->menuRepository->addAction(m_actionCommit);
    ui->menuRepository->addSeparator();
    ui->menuRepository->addAction(m_actionPush);
    ui->menuRepository->addAction(m_actionPull);
    
    // AI菜单
    m_actionAIConfig = new QAction("AI配置", this);
    
    ui->menuAI->addAction(m_actionAIConfig);
    
    // 帮助菜单
    m_actionAbout = new QAction("关于", this);
    
    ui->menuHelp->addAction(m_actionAbout);
    
    // 文件操作动作（右键菜单使用）
    m_actionStageFile = new QAction("暂存文件", this);
    m_actionUnstageFile = new QAction("取消暂存", this);
    m_actionDiscardChanges = new QAction("丢弃修改", this);
    m_actionViewDiff = new QAction("查看差异", this);
    
    // AI悬浮窗动作
    m_actionToggleAIFloatWidget = new QAction("AI助手悬浮窗", this);
    m_actionToggleAIFloatWidget->setCheckable(true);
    m_actionToggleAIFloatWidget->setChecked(false);
}

void MainWindow::setupToolBar()
{
    // 添加工具栏动作
    ui->mainToolBar->addAction(m_actionOpenRepo);
    ui->mainToolBar->addAction(m_actionCloneRepo);
    ui->mainToolBar->addAction(m_actionInitRepo);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actionCommit);
    ui->mainToolBar->addAction(m_actionPush);
    ui->mainToolBar->addAction(m_actionPull);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actionAIConfig);
    ui->mainToolBar->addAction(m_actionToggleAIFloatWidget);
}

void MainWindow::setupConnections()
{
    // 菜单和工具栏连接
    connect(m_actionOpenRepo, &QAction::triggered, this, &MainWindow::onActionOpenRepository);
    connect(m_actionCloneRepo, &QAction::triggered, this, &MainWindow::onActionCloneRepository);
    connect(m_actionInitRepo, &QAction::triggered, this, &MainWindow::onActionInitRepository);
    connect(m_actionCommit, &QAction::triggered, this, &MainWindow::onActionCommit);
    connect(m_actionPush, &QAction::triggered, this, &MainWindow::onActionPush);
    connect(m_actionPull, &QAction::triggered, this, &MainWindow::onActionPull);
    connect(m_actionSettings, &QAction::triggered, this, &MainWindow::onActionSettings);
    connect(m_actionAIConfig, &QAction::triggered, this, &MainWindow::onActionAIConfig);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    
    // Git管理器连接
    connect(m_gitManager, &GitManager::repositoryOpened, this, &MainWindow::onRepositoryOpened);
    connect(m_gitManager, &GitManager::repositoryClosed, this, &MainWindow::onRepositoryClosed);
    connect(m_gitManager, &GitManager::commandExecuted, this, &MainWindow::onCommandExecuted);
    connect(m_gitManager, &GitManager::errorOccurred, this, &MainWindow::onGitError);
    
    // AI管理器连接
    connect(m_aiManager, &AIManager::responseReady, this, &MainWindow::onAIResponse);
    connect(m_aiManager, &AIManager::errorOccurred, this, &MainWindow::onAIError);
    connect(m_aiManager, &AIManager::aiEnabledChanged, this, &MainWindow::onAIEnabledChanged);
    connect(m_aiManager, &AIManager::privacyModeChanged, this, &MainWindow::onPrivacyModeChanged);
    
    // 文件操作连接
    connect(m_actionStageFile, &QAction::triggered, this, &MainWindow::onActionStageFile);
    connect(m_actionUnstageFile, &QAction::triggered, this, &MainWindow::onActionUnstageFile);
    connect(m_actionDiscardChanges, &QAction::triggered, this, &MainWindow::onActionDiscardChanges);
    connect(m_actionViewDiff, &QAction::triggered, this, &MainWindow::onActionViewDiff);
    
    // AI悬浮窗连接
    connect(m_actionToggleAIFloatWidget, &QAction::triggered, this, [this](bool checked) {
        if (checked) {
            m_aiFloatWidget->showWidget();
        } else {
            m_aiFloatWidget->hideWidget();
        }
    });
    connect(m_aiFloatWidget, &AIFloatWidget::sendQuery, this, [this](const QString &query) {
        // 处理AI查询
        AIProvider::AIRequest request;
        request.type = AIProvider::ExplainGitCommand;
        request.content = query;
        m_currentAIRequestType = AIProvider::ExplainGitCommand;
        m_aiManager->sendRequest(request);
    });
    connect(m_aiFloatWidget, &AIFloatWidget::widgetClosed, this, [this]() {
        m_actionToggleAIFloatWidget->setChecked(false);
    });
    connect(m_aiFloatWidget, &AIFloatWidget::widgetShown, this, [this]() {
        m_actionToggleAIFloatWidget->setChecked(true);
    });
    connect(m_aiFloatWidget, &AIFloatWidget::widgetHidden, this, [this]() {
        m_actionToggleAIFloatWidget->setChecked(false);
    });
}

void MainWindow::onActionOpenRepository()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "打开Git仓库", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        if (m_gitManager->openRepository(dirPath)) {
            onRepositoryOpened(dirPath);
        }
    }
}

void MainWindow::onActionCloneRepository()
{
    bool ok;
    QString url = QInputDialog::getText(this, "克隆仓库", "请输入远程仓库URL:", QLineEdit::Normal, "", &ok);
    if (ok && !url.isEmpty()) {
        QString destPath = QFileDialog::getExistingDirectory(this, "选择目标目录", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!destPath.isEmpty()) {
            // 从URL中提取仓库名称
            QString repoName = url.split("/").last();
            if (repoName.endsWith(".git")) {
                repoName = repoName.left(repoName.length() - 4);
            }
            
            QString fullDestPath = destPath + QDir::separator() + repoName;
            
            if (m_gitManager->cloneRepository(url, fullDestPath)) {
                onRepositoryOpened(fullDestPath);
            }
        }
    }
}

void MainWindow::onActionInitRepository()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "选择目录", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        if (m_gitManager->initRepository(dirPath)) {
            onRepositoryOpened(dirPath);
        }
    }
}

void MainWindow::onActionCommit()
{
    // 生成AI提交信息建议
    QString aiSuggestion;
    if (m_aiEnabled && m_aiManager->getCurrentProvider()) {
        // 获取当前文件状态，用于生成提交信息
        QList<GitManager::FileInfo> fileStatus = m_gitManager->getFileStatus();
        QString diffContent;
        
        // 收集所有已修改文件的差异
        for (const GitManager::FileInfo &file : fileStatus) {
            if (file.status == GitManager::Modified || file.status == GitManager::Staged) {
                QString diff = m_gitManager->getDiff(file.path);
                if (!diff.isEmpty()) {
                    diffContent += "文件: " + file.path + "\n";
                    diffContent += diff + "\n\n";
                }
            }
        }
        
        if (!diffContent.isEmpty()) {
            // 调用AI生成提交信息
            AIProvider::AIRequest request;
            request.type = AIProvider::GenerateCommitMessage;
            request.content = diffContent;
            
            // 使用信号槽机制异步获取AI响应
            // 这里简化处理，直接使用默认提交信息
            aiSuggestion = "更新文件";
        }
    }
    
    bool ok;
    QString message = QInputDialog::getText(this, "提交", "请输入提交信息:", QLineEdit::Normal, aiSuggestion, &ok);
    if (ok && !message.isEmpty()) {
        if (m_gitManager->commit(message)) {
            QMessageBox::information(this, "提交成功", "提交已完成");
            updateFileStatus();
            updateCommitHistory();
        }
    }
}

void MainWindow::onActionPush()
{
    // 获取当前分支
    QList<GitManager::BranchInfo> branches = m_gitManager->getBranches();
    QString currentBranch;
    for (const GitManager::BranchInfo &branch : branches) {
        if (branch.isCurrent) {
            currentBranch = branch.name;
            break;
        }
    }
    
    if (currentBranch.isEmpty()) {
        QMessageBox::warning(this, "推送失败", "无法获取当前分支");
        return;
    }
    
    // 获取远程仓库
    QList<GitManager::RemoteInfo> remotes = m_gitManager->getRemotes();
    if (remotes.isEmpty()) {
        QMessageBox::warning(this, "推送失败", "没有配置远程仓库");
        return;
    }
    
    QString remoteName = remotes.first().name;
    
    if (m_gitManager->push(remoteName, currentBranch)) {
        QMessageBox::information(this, "推送成功", "推送已完成");
    }
}

void MainWindow::onActionPull()
{
    // 获取当前分支
    QList<GitManager::BranchInfo> branches = m_gitManager->getBranches();
    QString currentBranch;
    for (const GitManager::BranchInfo &branch : branches) {
        if (branch.isCurrent) {
            currentBranch = branch.name;
            break;
        }
    }
    
    if (currentBranch.isEmpty()) {
        QMessageBox::warning(this, "拉取失败", "无法获取当前分支");
        return;
    }
    
    // 获取远程仓库
    QList<GitManager::RemoteInfo> remotes = m_gitManager->getRemotes();
    if (remotes.isEmpty()) {
        QMessageBox::warning(this, "拉取失败", "没有配置远程仓库");
        return;
    }
    
    QString remoteName = remotes.first().name;
    
    if (m_gitManager->pull(remoteName, currentBranch)) {
        QMessageBox::information(this, "拉取成功", "拉取已完成");
        updateFileStatus();
        updateCommitHistory();
        updateBranchList();
    }
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this, "设置", "设置功能开发中");
}

void MainWindow::onActionAIConfig()
{
    AISettingDialog dialog(m_aiManager, this);
    dialog.exec();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, "关于SummerCake", "SummerCake Git GUI\n版本: 0.1.0\n基于Qt 6.10.0开发\n\n一个简单易用的Git GUI客户端，集成AI助手功能。");
}

void MainWindow::onRepositoryOpened(const QString &path)
{
    m_currentRepository = path;
    
    // 更新文件系统模型根路径
    ui->repoTreeView->setRootIndex(m_fileSystemModel->index(path));
    
    // 启用仓库相关功能
    ui->actionCommit->setEnabled(true);
    ui->actionPush->setEnabled(true);
    ui->actionPull->setEnabled(true);
    
    // 更新UI
    updateFileStatus();
    updateCommitHistory();
    updateBranchList();
    updateRemoteList();
    updateStatusBar();
    
    // 显示成功消息
    QMessageBox::information(this, "仓库已打开", "成功打开仓库: " + path);
}

void MainWindow::onRepositoryClosed()
{
    m_currentRepository.clear();
    
    // 禁用仓库相关功能
    ui->actionCommit->setEnabled(false);
    ui->actionPush->setEnabled(false);
    ui->actionPull->setEnabled(false);
    
    // 更新UI
    updateStatusBar();
}

void MainWindow::onCommandExecuted(const QString &command, const QString &output)
{
    // 可以在这里添加命令执行日志记录
    qDebug() << "Command executed:" << command;
    qDebug() << "Output:" << output;
}

void MainWindow::onGitError(const QString &error)
{
    QMessageBox::warning(this, "Git错误", error);
}

void MainWindow::onAIResponse(const AIProvider::AIResponse &response)
{
    if (response.success) {
        // 根据请求类型处理响应
        switch (m_currentAIRequestType) {
        case AIProvider::GenerateCommitMessage:
            m_aiCommitSuggestion = response.content;
            onAIGenerateCommitMessage(response);
            break;
        case AIProvider::CodeReview:
            ui->aiSuggestionView->setText(response.content);
            break;
        case AIProvider::ResolveConflict:
            ui->aiSuggestionView->setText(response.content);
            break;
        case AIProvider::ExplainGitCommand:
            ui->aiSuggestionView->setText(response.content);
            // 同时发送到AI悬浮窗
            if (m_aiFloatWidget && m_aiFloatWidget->isVisible()) {
                m_aiFloatWidget->onResponseReceived(response.content);
            }
            break;
        case AIProvider::RepositoryHealth:
            ui->aiSuggestionView->setText(response.content);
            break;
        case AIProvider::SmartCompletion:
            // 处理智能补全响应
            break;
        default:
            ui->aiSuggestionView->setText(response.content);
            break;
        }
    } else {
        ui->aiSuggestionView->setText("AI错误: " + response.errorMessage);
        // 同时发送到AI悬浮窗
        if (m_aiFloatWidget && m_aiFloatWidget->isVisible()) {
            m_aiFloatWidget->onErrorReceived(response.errorMessage);
        }
    }
}

void MainWindow::onAIGenerateCommitMessage(const AIProvider::AIResponse &response)
{
    // AI生成提交信息成功，更新建议
    m_aiCommitSuggestion = response.content;
    
    // 可以在这里添加通知用户AI建议已生成的逻辑
    qDebug() << "AI生成提交信息: " << m_aiCommitSuggestion;
}

void MainWindow::onAIError(const QString &error)
{
    QMessageBox::warning(this, "AI错误", error);
    ui->aiSuggestionView->setText("AI错误: " + error);
}

void MainWindow::onAIEnabledChanged(bool enabled)
{
    m_aiEnabled = enabled;
    updateStatusBar();
}

void MainWindow::onPrivacyModeChanged(bool enabled)
{
    m_privacyModeEnabled = enabled;
    updateStatusBar();
}

void MainWindow::updateFileStatus()
{
    // 获取文件状态
    QList<GitManager::FileInfo> fileStatus = m_gitManager->getFileStatus();
    
    // 更新模型
    m_fileStatusModel->setFileStatus(fileStatus);
    
    // 调整列宽
    ui->fileStatusView->resizeColumnsToContents();
    
    qDebug() << "更新文件状态完成，共" << fileStatus.size() << "个文件";
}

void MainWindow::updateCommitHistory()
{
    // 获取提交历史
    QList<GitManager::CommitInfo> commitHistory = m_gitManager->getCommitHistory();
    
    // 更新模型
    m_commitHistoryModel->setCommitHistory(commitHistory);
    
    // 调整列宽
    ui->commitHistoryView->resizeColumnsToContents();
    
    qDebug() << "更新提交历史完成，共" << commitHistory.size() << "个提交";
}

void MainWindow::updateBranchList()
{
    // 获取分支列表
    QList<GitManager::BranchInfo> branches = m_gitManager->getBranches();
    
    // 更新模型
    m_branchModel->setBranches(branches);
    
    qDebug() << "更新分支列表完成，共" << branches.size() << "个分支";
}

void MainWindow::updateRemoteList()
{
    // 获取远程仓库列表
    QList<GitManager::RemoteInfo> remotes = m_gitManager->getRemotes();
    
    // 这里可以将远程仓库信息显示在UI上，例如在状态栏或专门的视图中
    // 目前我们只记录日志
    qDebug() << "更新远程列表完成，共" << remotes.size() << "个远程仓库";
    for (const GitManager::RemoteInfo &remote : remotes) {
        qDebug() << "  远程仓库: " << remote.name << " - " << remote.url;
    }
}

void MainWindow::updateStatusBar()
{
    if (m_currentRepository.isEmpty()) {
        ui->branchLabel->setText("分支: 未打开仓库");
        ui->repoStatusLabel->setText("状态: 未打开仓库");
    } else {
        // 获取当前分支
        QList<GitManager::BranchInfo> branches = m_gitManager->getBranches();
        QString currentBranch = "未知";
        for (const GitManager::BranchInfo &branch : branches) {
            if (branch.isCurrent) {
                currentBranch = branch.name;
                break;
            }
        }
        
        ui->branchLabel->setText("分支: " + currentBranch);
        ui->repoStatusLabel->setText("状态: 已打开仓库");
    }
    
    ui->aiStatusLabel->setText("AI: " + (m_aiEnabled ? "已启用" : "已禁用") + 
                              (m_privacyModeEnabled ? " (隐私模式)" : ""));
}

void MainWindow::onFileStatusContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->fileStatusView->indexAt(pos);
    if (!index.isValid()) {
        return;
    }
    
    QMenu contextMenu(this);
    
    const GitManager::FileInfo &fileInfo = m_fileStatusModel->getFileInfo(index.row());
    
    // 根据文件状态添加不同的菜单项
    switch (fileInfo.status) {
    case GitManager::Modified:
        contextMenu.addAction(m_actionStageFile);
        contextMenu.addAction(m_actionDiscardChanges);
        contextMenu.addAction(m_actionViewDiff);
        break;
    case GitManager::Staged:
        contextMenu.addAction(m_actionUnstageFile);
        contextMenu.addAction(m_actionViewDiff);
        break;
    case GitManager::Untracked:
        contextMenu.addAction(m_actionStageFile);
        contextMenu.addAction(m_actionDiscardChanges);
        break;
    case GitManager::Deleted:
        contextMenu.addAction(m_actionStageFile);
        contextMenu.addAction(m_actionUnstageFile);
        break;
    case GitManager::Renamed:
        contextMenu.addAction(m_actionStageFile);
        contextMenu.addAction(m_actionUnstageFile);
        contextMenu.addAction(m_actionViewDiff);
        break;
    default:
        break;
    }
    
    contextMenu.exec(ui->fileStatusView->viewport()->mapToGlobal(pos));
}

void MainWindow::onActionStageFile()
{
    QModelIndexList selectedIndexes = ui->fileStatusView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }
    
    const GitManager::FileInfo &fileInfo = m_fileStatusModel->getFileInfo(selectedIndexes.first().row());
    
    if (m_gitManager->stageFile(fileInfo.path)) {
        updateFileStatus();
        QMessageBox::information(this, "操作成功", "文件已暂存");
    }
}

void MainWindow::onActionUnstageFile()
{
    QModelIndexList selectedIndexes = ui->fileStatusView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }
    
    const GitManager::FileInfo &fileInfo = m_fileStatusModel->getFileInfo(selectedIndexes.first().row());
    
    if (m_gitManager->unstageFile(fileInfo.path)) {
        updateFileStatus();
        QMessageBox::information(this, "操作成功", "文件已取消暂存");
    }
}

void MainWindow::onActionDiscardChanges()
{
    QModelIndexList selectedIndexes = ui->fileStatusView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }
    
    const GitManager::FileInfo &fileInfo = m_fileStatusModel->getFileInfo(selectedIndexes.first().row());
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认丢弃", "确定要丢弃对文件 \"" + fileInfo.path + "\" 的修改吗？此操作不可恢复。",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_gitManager->discardChanges(fileInfo.path)) {
            updateFileStatus();
            QMessageBox::information(this, "操作成功", "修改已丢弃");
        }
    }
}

void MainWindow::onActionViewDiff()
{
    QModelIndexList selectedIndexes = ui->fileStatusView->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        return;
    }
    
    const GitManager::FileInfo &fileInfo = m_fileStatusModel->getFileInfo(selectedIndexes.first().row());
    QString diff;
    
    if (fileInfo.status == GitManager::Staged) {
        diff = m_gitManager->getStagedDiff(fileInfo.path);
    } else {
        diff = m_gitManager->getDiff(fileInfo.path);
    }
    
    if (diff.isEmpty()) {
        diff = "没有差异";
    }
    
    ui->diffView->setPlainText(diff);
    ui->rightTabWidget->setCurrentWidget(ui->diffTab);
}
