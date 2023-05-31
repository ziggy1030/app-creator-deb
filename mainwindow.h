#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
//    //解析deb包，获取应用名、包名、版本号、架构等
//    void AnalyticDeb(QString);


//    //解析snap包，获取应用名、包名、版本号、架构等
//    void AnalyticSnap(QString);


//    QString CallCMD(QString);


    //通过应用名称下载snap包
//    QString CurlSnap(QString);
//    void WgetPkg(QString);

//    void UnsquashfsSnap(QString);
    void InitStructure(QString path);

    void ClearAllUI();





    //点击选择文件
    QString GetFilePath();

    //点击选择输出路径
    QString GetOutPutPath();


signals:
    void sendSnapAppName(QString name,QString m_architecture);

    void sendDebPkgName(QString path);

    void sendOutPutPath(QString path);

    void sendBuildInfo(QMap<QString,QString> build_info_map);

    void sendAppName(QString appname);



private slots:
    void on_pushButton_selectdeb_clicked();

    void on_pushButton_searchsnap_clicked();

    void on_pushButton_selectoutpath_clicked();

    void on_pushButton_convert_clicked();


private:
    Ui::MainWindow *ui;
    QString m_debPath;
    QString m_appName;
    QString m_architecture;
    QString m_url;
    QString m_version;
    QString m_name;
    QString m_publisher;
    QString m_homepage;
    QString m_summary;
    QString m_outPath;
    QString m_type;
};

#endif // MAINWINDOW_H
