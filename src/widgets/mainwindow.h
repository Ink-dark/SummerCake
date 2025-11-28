#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QFileSystemModel>

#include "git/gitmanager.h"
#include "ai/aimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 菜单和工具栏操作
    void onActionOpenRepository();
    void onActionCloneRepository();
    void onActionInitRepository();
    void onActionCommit();
    void onActionPush();
    void onActionPull();
    void onActionSettings();
    void onActionAIConfig();
    void onActionAbout();

    // 文件操作
    void onFileStatusContextMenu(const QPoint &pos);
    void onActionStageFile();
    void onActionUnstageFile();
    void onActionDiscardChanges();
    void onActionViewDiff();

    // Git事件处理
    void onRepositoryOpened(const QString &path);
    void onRepositoryClosed();
    void onCommandExecuted(const QString &command, const QString &output);
    void onGitError(const QString &error);

    // AI事件处理
    void onAIResponse(const AIProvider::AIResponse &response);
    void onAIError(const QString &error);
    void onAIEnabledChanged(bool enabled);
    void onPrivacyModeChanged(bool enabled);
    void onAIGenerateCommitMessage(const AIProvider::AIResponse &response);

    // UI更新
    void updateFileStatus();
    void updateCommitHistory();
    void updateBranchList();
    void updateRemoteList();

private:
    void setupUI();
    void setupMenus();
    void setupToolBar();
    void setupConnections();
    void updateStatusBar();

    Ui::MainWindow *ui;
    
    // 核心管理器
    GitManager *m_gitManager;
    AIManager *m_aiManager;
    
    // UI组件
    QSplitter *m_mainSplitter;
    QSplitter *m_leftSplitter;
    QSplitter *m_rightSplitter;
    
    // 左侧面板
    QTabWidget *m_leftTabWidget;
    QTreeView *m_repoTreeView;
    QListView *m_branchListView;
    QListView *m_tagListView;
    
    // 中间面板
    QTabWidget *m_centerTabWidget;
    QTableView *m_fileStatusView;
    QTableView *m_commitHistoryView;
    
    // 右侧面板
    QTabWidget *m_rightTabWidget;
    QTextEdit *m_diffView;
    QTextEdit *m_commitDetailView;
    QTextEdit *m_aiSuggestionView;
    
    // 底部状态栏
    QLabel *m_branchLabel;
    QLabel *m_repoStatusLabel;
    QLabel *m_aiStatusLabel;
    
    // 数据模型
    QFileSystemModel *m_fileSystemModel;
    FileStatusModel *m_fileStatusModel;
    CommitHistoryModel *m_commitHistoryModel;
    BranchModel *m_branchModel;
    RemoteModel *m_remoteModel;
    
    // 菜单和工具栏
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_repoMenu;
    QMenu *m_branchMenu;
    QMenu *m_remoteMenu;
    QMenu *m_aiMenu;
    QMenu *m_helpMenu;
    
    QToolBar *m_mainToolBar;
    
    // 动作
    QAction *m_actionOpenRepo;
    QAction *m_actionCloneRepo;
    QAction *m_actionInitRepo;
    QAction *m_actionCommit;
    QAction *m_actionPush;
    QAction *m_actionPull;
    QAction *m_actionSettings;
    QAction *m_actionAIConfig;
    QAction *m_actionAbout;
    
    // 文件操作动作
    QAction *m_actionStageFile;
    QAction *m_actionUnstageFile;
    QAction *m_actionDiscardChanges;
    QAction *m_actionViewDiff;
    QAction *m_actionToggleAIFloatWidget;
    
    // AI悬浮窗
    AIFloatWidget *m_aiFloatWidget;
    
    // 状态
    QString m_currentRepository;
    bool m_aiEnabled;
    bool m_privacyModeEnabled;
    AIProvider::AIRequestType m_currentAIRequestType;
    QString m_aiCommitSuggestion;
};
#endif // MAINWINDOW_H
