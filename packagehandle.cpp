#include "packagehandle.h"
#include <QDebug>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QCoreApplication>
#include <QFile>
#include <QDirIterator>
#include <QDir>


PackageHandle::PackageHandle(QObject *parent) : QObject(parent)
    ,m_deb_src_path(QDir::currentPath()+"/debsrcfile")
  ,m_snap_src_path(QDir::currentPath()+"/snapsrcfile")
{
//    HttpsDownload* download_snap = new HttpsDownload;
//    QThread* t1 = new QThread;
//    download_snap->moveToThread(t1);

//    connect(this,&PackageHandle::sendSnapInfo,download_snap,&HttpsDownload::DownLoading);
//    connect(this,&PackageHandle::sendSnapInfo,this,[=](QMap<QString, QString> info_map){
//        t1->start();
//    });
//    connect(this,&PackageHandle::deleteLater,this,[=]{
//        t1->quit();
//        t1->wait();
//        t1->deleteLater();

//        download_snap->deleteLater();

//    });


}

QByteArray PackageHandle::RequestSnap(QString app_name)
{
//    实现此功能： curl -H 'Snap-Device-Series: 16' http://api.snapcraft.io/v2/snaps/info/asbru

      const QString snapcraftUrl = "http://api.snapcraft.io/v2/snaps/info/" + app_name.toLower();
      const QByteArray headerName = "Snap-Device-Series";
      const QByteArray headerValue = "16";

    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this);

    QNetworkRequest request;
    request.setUrl(QUrl(snapcraftUrl));
    request.setRawHeader(headerName, headerValue);


    QNetworkReply* reply = networkManager->get(request);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();       //block until finish

    if (reply->error()) {
        qInfo() << "Error:" << reply->errorString();
        emit responseErr();
        return "";
    }

    QByteArray responseByte = reply->readAll();
//    qInfo() << "Response:" << responseByte;

    reply->deleteLater();
    networkManager->deleteLater();


    return responseByte;





}

QMap<QString, QString> PackageHandle::FilteringInfo(QByteArray snap_info,QString arch_)
{
    QMap<QString, QString> info_map_;
    QJsonDocument doc = QJsonDocument::fromJson(snap_info.toStdString().c_str());
    QJsonObject JSON_obj = doc.object();

    QJsonObject channel_map;
    QString ar;
    //type获取QJsonObject的value个数
    for (int i = 0; i < JSON_obj.value("channel-map").type() - 1; i++) {
        channel_map = JSON_obj.value("channel-map")[i].toObject();
         ar = channel_map.value("channel").toObject().value("architecture").toString();
         if(ar == arch_){
             info_map_["architecture"] = ar;
             info_map_["url"] = channel_map.value("download").toObject().value("url").toString();
             info_map_["size"] =  "约 " + QString::number(channel_map.value("download").toObject().value("size").toInt()/1024/1024)+" MB";
             info_map_["version"] = channel_map.value("version").toString();
             info_map_["name"] = JSON_obj.value("name").toString();
             m_appname = info_map_["name"];
             m_version = info_map_["version"];
             m_arch = info_map_["architecture"];

             QJsonObject snap = JSON_obj.value("snap").toObject();
             info_map_["publisher"] = snap.value("publisher").toObject().value("username").toString();
             info_map_["homepage"] = snap.value("store-url").toString();
             info_map_["summary"] = snap.value("summary").toString();
             m_homepage = info_map_["homepage"];
             m_description = info_map_["summary"];
             break;
         }
    }

    return info_map_;
}

void PackageHandle::DownloadSnap(QString snap_url, QString name, QString architecture)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

//        QUrl url("https://api.snapcraft.io/api/v1/snaps/download/oHNBJUD1kdfzgjkRHBz7kDGcyfgzbMD1_15.snap");
    QUrl url(snap_url);

    QNetworkRequest request;
    request.setUrl(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    qInfo() << url;
    QNetworkReply *reply = manager->get(request);

    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();       //block until finish
    m_filename = name + "_" + architecture + ".snap";

    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qInfo() << "Failed to open file" << m_filename;
        return;
    }
    QByteArray responseByte = reply->readAll();
    qInfo() << "request finish!";
    file.write(responseByte);
    file.close();
    qInfo() << "write finish!";

    reply->deleteLater();
    manager->deleteLater();
}



void PackageHandle::UnsquashfsSnap(QString snap_pkg)
{
    qInfo() << "Extracting snap file...";
    QString cmdstr_ = "rm -rf snapsrcfile && unsquashfs -f -d ";
    cmdstr_.append("snapsrcfile ");
    cmdstr_.append(snap_pkg);
    CallCMD(cmdstr_);
    qInfo() << "unsquashfssnap over";

}

void PackageHandle::DebHand(QString path)
{
    ExtractDeb(path);
    ReadDebian(m_deb_src_path);

}

void PackageHandle::SnapHand(QString app_name, QString arch)
{
    QMap<QString, QString> map_;
    QByteArray res_ = RequestSnap(app_name);
    if(res_.isEmpty()){
        return;
    }
    map_ = FilteringInfo(res_,arch);
    emit sendSnapInfo(map_);
    //TODO： 此处加一个判断，避免后续map为空时，仍然显示control内容。这里可以注释掉这个判断，进一步定位问题出现原因，我懒得找了
    if(map_.isEmpty()){
        return;
    }
    //1234567
//    DownloadSnap(map_["url"], map_["name"], map_["architecture"]);
    m_filename = map_["name"] + "_" + map_["architecture"] + ".snap";
    UnsquashfsSnap(m_filename);
    ReadDebian(m_snap_src_path);
}


void PackageHandle::ExtractDeb(QString path)
{

    if(path.isEmpty())
        return;
    QString pkgtype = CallCMD("file "+path);
    if(!pkgtype.contains("Debian binary package")){
        qInfo()<<"包类型不合法："<<pkgtype;
        emit typeErr(pkgtype);
        return;
    }
    qInfo() << "Extracting deb file...";
    const QString cmdstr_ = "rm -rf debsrcfile && dpkg-deb -R " + path + " debsrcfile";
    CallCMD(cmdstr_);
    qInfo() << "Extractingdeb over";
}

void PackageHandle::ReadDebian(QString path)
{
    ReadControlFile(path);
    ReadDesktopFile(path);
    ReadInfoFile(path);
}

void PackageHandle::ReadControlFile(QString path)
{
            QByteArray control_byte;
    if(path.contains("debsrcfile")){
        QString control_path = SearchFile(path,"control");
        qInfo() << "control_path"<<control_path;

        if(!control_path.isEmpty()){
            control_byte = ReadText(control_path);
        }
    }else {
    control_byte.append(QString("Package: %1\n").arg(m_pkgname));
    control_byte.append(QString("Version: %1\n").arg(m_version));
    control_byte.append(QString("Architecture: %1\n").arg(m_arch));
    control_byte.append("Maintainer: uos uos@uniontech.com\n");
    control_byte.append("Section: net\n");
    control_byte.append("Priority: optional\n");
    control_byte.append(QString("Homepage: %1\n").arg(m_homepage));
    control_byte.append(QString("Description: %1\r\n").arg(m_description));
    control_byte.append("\n");
    }
    emit sendControl(control_byte);
}

void PackageHandle::ReadDesktopFile(QString path)
{
    QString desktop_path = SearchFile(path,".desktop");
    qInfo() << "desktop_path"<<desktop_path;
    QByteArray desktop_byte;
    if(!desktop_path.isEmpty()){
        QString str(ReadText(desktop_path));
        QStringList list = str.split(QRegExp("[\n]"),QString::SkipEmptyParts);
        for (int i = 0; i < list.size(); i++) {
            if (list[i].contains("Exec=")) {
                QString yamlstr = ReadText(path+"/meta/snap.yaml");
                QStringList yamllist = yamlstr.split(QRegExp("[\n]"),QString::SkipEmptyParts);
                for (int j = 0; j < yamllist.size(); j++) {
                    if(yamllist[j].contains("command:")){
                        //TODO: 如果路径有带参数xxx xxx xxx格式，需要特殊处理
                        //Exec=/opt/apps//files/  bin/electron-launch $SNAP/usr/lib/slack/slack --no-sandbox
                        if(yamllist[j].contains("$SNAP/")){
                            m_execPath = "/opt/apps/" + m_map["pkg_name"] + "/files/"+yamllist[j].split(":")[1].split("$SNAP/")[1].trimmed();
                        }else {
                            m_execPath = "/opt/apps/" + m_map["pkg_name"] + "/files/"+yamllist[j].split(":")[1].trimmed();

                        }
                    }
                }

                list[i] = "Exec=" + m_execPath;//这边需要设置可执行程序的路径，不是desktop的路径，需要修改
            }
            if(list[i].contains("Icon=")){
                //                //Icon=${SNAP}/meta/gui/icon.png
                m_icon_abpath = list[i].split("=")[1].replace("${SNAP}",path);
                QFileInfo fileinfo_(m_icon_abpath);

                m_iconname = fileinfo_.fileName();
                //TODO: 重新编辑路径，并且要变更textchange的路径
//                list[i].replace("${SNAP}","/opt/apps/" + m_map["pkg_name"] + "/entries/icons/");
                list[i] = "Icon=/opt/apps/" + m_map["pkg_name"] + "/entries/icons/" + m_iconname;
            }
            desktop_byte.append(list[i]);
            desktop_byte.append("\n");
        }
    }else {
        desktop_byte.append("[Desktop Entry]\n");
        desktop_byte.append("Type=Application\n");
        desktop_byte.append(QString("Name=%1\n").arg(m_appname));
        desktop_byte.append("Exec=\n");
        desktop_byte.append("Icon=\n");
        desktop_byte.append("Terminal=false\n");
    }
    emit sendDesktop(m_desktop_name, desktop_byte);

}

void PackageHandle::ReadInfoFile(QString path)
{
    QString UOSinfo_path = SearchFile(path,"info");
    qInfo() << "UOSinfo_path"<<UOSinfo_path;
    QByteArray UOSinfo_byte;
    if(!UOSinfo_path.isEmpty()){

        UOSinfo_byte = ReadText(UOSinfo_path);

    }else {
        UOSinfo_byte.append("{\n");
        UOSinfo_byte.append(QString(" \"appid\": \"%1\",\n").arg(m_pkgname));
        UOSinfo_byte.append(QString(" \"name\": \"%1\",\n").arg(m_appname));
        UOSinfo_byte.append(QString(" \"version\": \"%1\",\n").arg(m_version));
        UOSinfo_byte.append(QString(" \"arch\": [\"%1\"],\n").arg(m_arch));
        UOSinfo_byte.append(" \"permissions\": {\n");
        UOSinfo_byte.append("  \"autostart\": false,\n");
        UOSinfo_byte.append("  \"notification\": false,\n");
        UOSinfo_byte.append("  \"trayicon\": false,\n");
        UOSinfo_byte.append("  \"clipboard\": false,\n");
        UOSinfo_byte.append("  \"account\": false,\n");
        UOSinfo_byte.append("  \"bluetooth\": false,\n");
        UOSinfo_byte.append("  \"camera\": false,\n");
        UOSinfo_byte.append("  \"audio_record\": false,\n");
        UOSinfo_byte.append("  \"installed_apps\": false\n");
        UOSinfo_byte.append("  }\n");
        UOSinfo_byte.append("}\n");
    }
    emit sendUOSInfo(UOSinfo_byte);

}




QString PackageHandle::CallCMD(QString commands)
{
    qInfo() << "Running:" << commands;
    QProcess process;
    process.start("bash", QStringList() << "-c" << commands);
    process.waitForFinished();
    qInfo() << "CallCMD Over!";
    return process.readAllStandardOutput();
}

QString PackageHandle::SearchFile(QString path, QString filename)
{

    QDirIterator iter(path,
                      QDir::Files | QDir::NoSymLinks,
                      QDirIterator::Subdirectories);
    while(iter.hasNext())
    {
        iter.next();

        if(filename == ".desktop"){
            if(iter.fileName().endsWith(filename)){
                m_desktop_name = iter.fileName();
                qInfo() << "fileName:" << m_desktop_name;  // 只有文件名
                QFileInfo fileInfo = iter.fileInfo();
                QString absoluteFilePath = fileInfo.absoluteFilePath(); //这个和iter.filePath()结果一样
                return absoluteFilePath;
            }
        }
        if(iter.fileName() == filename){
            qInfo() << "fileName:" << iter.fileName();  // 只有文件名
            QFileInfo fileInfo = iter.fileInfo();
            QString absoluteFilePath = fileInfo.absoluteFilePath(); //这个和iter.filePath()结果一样
            return absoluteFilePath;
        }

    }
    return "";

}

QByteArray PackageHandle::ReadText(QString path)
{
    QFile file(path);


    QByteArray bytes;
    if(!file.exists()) //文件不存在则退出
       {
           qDebug()<<"file not exist";
           return "";

       }
       if(file.open(QFile::ReadOnly))
       {
           bytes = file.readAll();
//           qInfo() <<bytes;
       }

       file.close();
       return bytes;
}

void PackageHandle::Write2File(QString filepath_, QString text_)
{
    QFile file(filepath_); // 文件路径
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qInfo() << "Failed to open file!";
        return;
    }

    QTextStream out(&file); // 新建一个输出流
    out << text_; // 写入文件
    out.flush(); // 刷新缓冲区
    file.close();
}

void PackageHandle::BuildDebPkg(QMap<QString,QString> map_)
{
    m_map = map_;
    QString path_;


    if(m_map["type"] == "deb"){
        path_ = "debsrcfile";
    }else if (m_map["type"] == "snap") {
        path_ = "snapsrcfile";
    }else {
        return;
    }
    //1. 若info目录不存在（不是UOS的包），则InitNewDir
    if(SearchFile(path_,"info").isEmpty()){
        qInfo() <<"无info文件";
        InitNewDir(path_);
    }
    //2. 写文件信息
        //control写到path_/DEBIAN
        Write2File(path_+"/DEBIAN/control",map_["control"]);
        //info写到path_ + "/opt/apps/" + m_map["pkg_name"]+"/info"
        Write2File(path_ + "/opt/apps/" + m_map["pkg_name"]+"/info",map_["info"]);

        //desktop写到path_ + "/opt/apps/" + m_map["pkg_name"]+"/entries/applications"
        QFileInfo fileInfo(SearchFile(path_,".desktop"));
        qInfo() << fileInfo.fileName();
        Write2File(path_ + "/opt/apps/" + m_map["pkg_name"]+"/entries/applications/" + fileInfo.fileName(),map_["desktop"]);


        //TODO:
        //3. 删除不必要的文件{data-dir,gnome-platform,lib,meta,scripts,usr,*.sh}


        //4. 打包dpkg-deb -b
        QString pkgarch;
        if(m_map["arch"] == "amd64"){
            pkgarch = "X86";
        }else if (m_map["arch"] == "arm64") {
            pkgarch = "ARM";
        }else {
            qInfo() << "架构有误" << __LINE__ << __FUNCTION__;
        }
        QString buildStr = QString("dpkg-deb -b %1 %2").arg(path_).arg(m_map["outPath"]).arg(m_map["debname"]);
        CallCMD(buildStr);
        emit BuildFinish();
}

void PackageHandle::InitNewDir(QString path_)
{
    //1. 新建目录
    QDir srcDir(path_);
    srcDir.mkpath("/opt/apps/" + m_map["pkg_name"]);

    QDir destDir(path_ + "/opt/apps/" + m_map["pkg_name"]);
    destDir.mkpath("entries/icons");
    destDir.mkpath("entries/applications");
    destDir.mkpath("files");

    //2. 移动旧目录到新目录中
    //2. 移动旧目录到新目录中
    QString cpiconStr = QString("cp %1 %2/opt/apps/%3/entries/icons/").arg(m_icon_abpath).arg(path_).arg(m_map["pkg_name"]);
    CallCMD(cpiconStr);

    QString mvStr = QString("cd %1 && mv `ls |grep -v opt` opt/apps/%2/files/").arg(path_).arg(m_map["pkg_name"]);
    qInfo() << mvStr;
    QString x = CallCMD(mvStr);
    qInfo() << x;
    srcDir.mkpath("DEBIAN");
}



