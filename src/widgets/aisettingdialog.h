#ifndef AISETTINGDIALOG_H
#define AISETTINGDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>

#include "ai/aimanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class AISettingDialog;
}
QT_END_NAMESPACE

class AISettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AISettingDialog(AIManager *aiManager, QWidget *parent = nullptr);
    ~AISettingDialog();

private slots:
    void onProviderChanged(int index);
    void onSaveSettings();
    void onTestConnection();
    void onResetSettings();

private:
    void setupUI();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void updateProviderSettings();

    Ui::AISettingDialog *ui;
    AIManager *m_aiManager;
    
    // UI组件
    QComboBox *m_providerComboBox;
    QCheckBox *m_aiEnabledCheckBox;
    QCheckBox *m_privacyModeCheckBox;
    
    // 提供商配置
    QGroupBox *m_providerSettingsGroup;
    QFormLayout *m_providerSettingsLayout;
    QMap<QString, QWidget*> m_providerSettingsWidgets;
    
    // 按钮
    QPushButton *m_saveButton;
    QPushButton *m_testButton;
    QPushButton *m_resetButton;
    QPushButton *m_cancelButton;
    
    // 当前选择的提供商
    QString m_currentProvider;
};

#endif // AISETTINGDIALOG_H
