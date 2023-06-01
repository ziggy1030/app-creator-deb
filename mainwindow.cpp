#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <packagehandle.h>
#include <builddeb.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineEdit_snapname->setText("lightning-talk-gong");
    PackageHandle* pkg = new PackageHandle;

    //主窗口发送deb包路径，deb对象接收并处理
    connect(this,&MainWindow::sendDebPkgName,pkg,&PackageHandle::DebHand);
    connect(pkg,&PackageHandle::typeErr,this,[=]{
        QMessageBox::critical(this, "Warning", "包类型不合法", QMessageBox::Ok);
        ui->lineEdit_selectdeb->clear();
    });


    //主窗口发送应用名，snap对象接收并请求
    connect(this,&MainWindow::sendSnapAppName,pkg,&PackageHandle::SnapHand);
    //snap对象发送请求结果，主窗口接收
    connect(pkg,&PackageHandle::responseErr,this,[=]{


        QMessageBox::critical(this, "Warning", "搜索无结果", QMessageBox::Ok);
        ClearAllUI();
        return ;
    });
    connect(pkg,&PackageHandle::sendSnapInfo,this,[=](QMap<QString, QString> info_map_){
        if(info_map_.isEmpty()){
            qInfo() << info_map_;
            QMessageBox::critical(this, "Warning", "搜索不到此架构", QMessageBox::Ok);
            ClearAllUI();

            return;
        }
        ClearAllUI();
        //界面内容填充

        m_architecture = info_map_["architecture"];
        m_url = info_map_["url"];
        m_version = info_map_["version"];
        m_name = info_map_["name"];
        m_publisher = info_map_["publisher"];
        m_homepage = info_map_["homepage"];
        m_summary = info_map_["summary"];


        ui->label_size2->setText(info_map_["size"]);
        ui->label_developer->setText(info_map_["publisher"]);
        ui->label_snappkg->setText(info_map_["homepage"]);
        ui->label_snapdownurl->setText(info_map_["url"]);
        ui->lineEdit_appname->setText(m_name);
        ui->lineEdit_version->setText(m_version);
        if(info_map_["architecture"] == "amd64"){
            ui->lineEdit_arch->setText("X86");
        }else if (info_map_["architecture"] == "arm64") {
            ui->lineEdit_arch->setText("ARM");
        }else {
            qInfo() << "架构错误" << __LINE__ << __FUNCTION__;
        }

    });

    connect(this,&MainWindow::sendBuildInfo,pkg,&PackageHandle::BuildDebPkg);

    connect(pkg,&PackageHandle::sendControl,this,[=](QByteArray bytes){
//        ClearAllUI();
        ui->textEdit_control->setText(bytes);
        QString str(bytes);

        QStringList list = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

        for (int i = 0; i < list.size(); i++) {
            if (list[i].contains("Package")) {
                if(ui->lineEdit_pkgname->text().isEmpty()){
                    ui->lineEdit_pkgname->setText(list[i].split(": ")[1]);
                }
            }
            if (list[i].contains("Version")) {
                if(ui->lineEdit_version->text().isEmpty()){
                    ui->lineEdit_version->setText(list[i].split(": ")[1]);
                }
            }
            if (list[i].contains("Architecture")) {
                if(ui->lineEdit_arch->text().isEmpty()){
                    ui->lineEdit_arch->setText(list[i].split(": ")[1]);
                }
            }
        }

    });
    connect(pkg,&PackageHandle::sendDesktop,this,[=](QString desktop_name, QByteArray bytes){
        ui->textEdit_desktop->setText(bytes);
        ui->lineEdit_appname->setText(desktop_name.split(".")[0]);
    });
    connect(pkg,&PackageHandle::sendUOSInfo,this,[=](QByteArray bytes){
        ui->textEdit_info->setText(bytes);
    });

    //包名变动时
    connect(ui->lineEdit_pkgname,&QLineEdit::textChanged,this,[=]{
        qInfo() << "lineEdit_pkgname-textchange";
        //同步修改输出deb名称
        if(!ui->lineEdit_pkgname->text().isEmpty() &&
                !ui->lineEdit_version->text().isEmpty()){
            ui->lineEdit_debname->setText(ui->lineEdit_pkgname->text() + "_" +
                                          ui->lineEdit_version->text() + "_" +
                                          ui->lineEdit_arch->text() + ".deb");
        }

        //同步修改control文件
        QStringList controllist = ui->textEdit_control->toPlainText().split(QRegExp("[\n]"),QString::SkipEmptyParts);
        ui->textEdit_control->clear();
        for (int i = 0; i < controllist.size(); i++) {
            if (controllist[i].contains("Package")) {
                controllist[i] = QString("Package: %1").arg(ui->lineEdit_pkgname->text());
            }
            ui->textEdit_control->append(controllist[i]);
        }
        ui->textEdit_control->append("\n");

        //同步修改desktop文件
        QStringList desktoplist = ui->textEdit_desktop->toPlainText().split(QRegExp("[\n]"),QString::SkipEmptyParts);
        ui->textEdit_desktop->clear();
        for (int i = 0; i < desktoplist.size(); i++) {
            if (desktoplist[i].contains("Icon=")) {
                if (desktoplist[i].contains("/")) {
                    QFileInfo file(desktoplist[i].split("=")[1]);
                    desktoplist[i] = QString("Icon=/opt/apps/%1/entries/icons/%2").arg(ui->lineEdit_pkgname->text()).arg(file.fileName());
                }
            }
//TODO: 此处需要处理slack包的样式
            if (desktoplist[i].contains("Exec=")) {
                QFileInfo file(desktoplist[i].split("=")[1]);
                //  Exec=/opt/apps//files/usr/lib/slack/slack --no-sandbox
                QString execStr = desktoplist[i].split("/files/")[1];
                desktoplist[i] = QString("Exec=/opt/apps/%1/files/%2").arg(ui->lineEdit_pkgname->text()).arg(execStr);

            }

            ui->textEdit_desktop->append(desktoplist[i]);
        }

        //同步修改info文件
        QStringList infolist = ui->textEdit_info->toPlainText().split(QRegExp("[\n]"),QString::SkipEmptyParts);
        ui->textEdit_info->clear();
        for (int i = 0; i < infolist.size(); i++) {
            if (infolist[i].contains("appid")) {
                infolist[i] = QString(" \"appid\": \"%1\",").arg(ui->lineEdit_pkgname->text());
            }
            ui->textEdit_info->append(infolist[i]);
        }

    });

    //版本变动时，输出的deb包名的版本号也随之变动
    connect(ui->lineEdit_version,&QLineEdit::textChanged,ui->lineEdit_debname,[=]{
        qInfo() << "lineEdit_version-textchange";
        if(!ui->lineEdit_pkgname->text().isEmpty() &&
                !ui->lineEdit_version->text().isEmpty()){
            ui->lineEdit_debname->setText(ui->lineEdit_pkgname->text() + "_" +
                                          ui->lineEdit_version->text() + "_" +
                                          ui->lineEdit_arch->text() +".deb");
        }
    });

    //架构变动时，输出的deb包名的架构也随之变动
    connect(ui->lineEdit_arch,&QLineEdit::textChanged,ui->lineEdit_debname,[=]{
        qInfo() << "lineEdit_arch-textchange";
        if(!ui->lineEdit_pkgname->text().isEmpty() &&
                !ui->lineEdit_version->text().isEmpty()){
            ui->lineEdit_debname->setText(ui->lineEdit_pkgname->text() + "_" +
                                          ui->lineEdit_version->text() + "_" +
                                          ui->lineEdit_arch->text() +".deb");
        }
    });
    connect(pkg,&PackageHandle::BuildFinish,this,[=]{
        QMessageBox::information(this, "Info", "打包完成", QMessageBox::Ok);
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ClearAllUI()
{
    ui->lineEdit_appname->clear();
    ui->lineEdit_pkgname->clear();
    ui->lineEdit_version->clear();
    ui->textEdit_desktop->clear();
    ui->textEdit_control->clear();
    ui->textEdit_info->clear();
    ui->label_snappkg->clear();
    ui->label_snapdownurl->clear();
    ui->lineEdit_arch->clear();
}





QString MainWindow::GetFilePath()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("文件选取"),QDir::homePath(),tr("deb文件(*deb)"));
    return fileName;

}

QString MainWindow::GetOutPutPath()
{
    QString dirName = QFileDialog::getExistingDirectory(this,"选择一个目录",QDir::homePath(),QFileDialog::ShowDirsOnly);
    return dirName;
}

void MainWindow::on_pushButton_selectdeb_clicked()
{
    m_type = "deb";
    m_debPath = GetFilePath();
    qInfo() << "选择的deb包：" << m_debPath;
    if(m_debPath.isEmpty()){
        return;
    }

    ui->lineEdit_selectdeb->setText(m_debPath);
    if(m_debPath.contains("amd") || m_debPath.contains("X86"))
        ui->lineEdit_arch->setText("X86");
    else if (m_debPath.contains("arm") || m_debPath.contains("ARM")) {
        ui->lineEdit_arch->setText("ARM");
    }else {
        qInfo() << "需要手动选择架构";
}

    ClearAllUI();
    emit sendDebPkgName(m_debPath);
    ui->lineEdit_snapname->clear();
    ui->label_snappkg->clear();
}

void MainWindow::on_pushButton_searchsnap_clicked()
{



    m_appName = ui->lineEdit_snapname->text();
    if(ui->radioButton_x86->isChecked() ){
        m_architecture = "amd64";
    }
    if (ui->radioButton_arm->isChecked()) {
        m_architecture = "arm64";
    }
    if(m_appName.isEmpty() || m_architecture.isEmpty()){
        QMessageBox::critical(this, "Warning", "完善搜索信息", QMessageBox::Ok);
        return;
    }
    m_type = "snap";
    qInfo() << "正在搜索" << m_architecture <<"架构的snap包：" << m_appName;

    ClearAllUI();
    emit sendSnapAppName(m_appName,m_architecture);
    ui->lineEdit_selectdeb->clear();

}

void MainWindow::on_pushButton_selectoutpath_clicked()
{
    m_outPath = GetOutPutPath();
    qInfo() << "输出路径：" << m_outPath;
    ui->lineEdit_outpath->setText(m_outPath);
//    emit sendOutPutPath(m_outPath);

}

void MainWindow::on_pushButton_convert_clicked()
{
    if(ui->lineEdit_pkgname->text().isEmpty()){
        QMessageBox::critical(this, "Warning", "包名为空", QMessageBox::Ok);
        return;
    }

    if(ui->lineEdit_outpath->text().isEmpty()){
        QMessageBox::critical(this, "Warning", "输出路径为空", QMessageBox::Ok);
        return;
    }

    QMap<QString,QString> build_info_;

    build_info_["app_name"] = ui->lineEdit_appname->text();
    build_info_["version"] = ui->lineEdit_version->text();
    build_info_["arch"] = ui->lineEdit_arch->text();
    build_info_["pkg_name"] = ui->lineEdit_pkgname->text();
    build_info_["outPath"] = ui->lineEdit_outpath->text();
    build_info_["debPath"] = ui->lineEdit_selectdeb->text();
    build_info_["type"] = m_type;
    build_info_["control"] = ui->textEdit_control->toPlainText();
    build_info_["info"] = ui->textEdit_info->toPlainText();
    build_info_["desktop"] = ui->textEdit_desktop->toPlainText();
    build_info_["debname"] = ui->lineEdit_debname->text();

    emit sendBuildInfo(build_info_);
}
