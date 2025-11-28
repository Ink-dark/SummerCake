#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include "widgets/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("SummerCake");
    a.setApplicationVersion("0.1.0");
    a.setOrganizationName("Ink-dark");
    
    // 加载中文翻译
    QTranslator translator;
    QString translationPath = QDir::currentPath() + "/resources/translations/summercake_zh_CN.qm";
    
    // 尝试从应用程序目录加载翻译文件
    if (!translator.load(translationPath)) {
        // 如果失败，尝试从资源文件加载
        if (!translator.load("summercake_zh_CN", ":/resources/translations")) {
            qDebug() << "无法加载翻译文件";
        } else {
            a.installTranslator(&translator);
        }
    } else {
        a.installTranslator(&translator);
    }
    
    // 创建主窗口
    MainWindow w;
    w.show();
    
    return a.exec();
}
