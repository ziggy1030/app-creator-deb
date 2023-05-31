#include "httpsdownload.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QFile>
#include <QThread>



HttpsDownload::HttpsDownload()
{

}

void HttpsDownload::DownLoading(QMap<QString, QString> info_map)
{
    qInfo() << "线程地址：" << QThread::currentThread();
    //    QString cmdstr_ = "wget -cO ";
    //    cmdstr_.append(name+".snap ");
    //    cmdstr_.append(snap_url);
    //    CallCMD(cmdstr_);
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &QCoreApplication::quit);

//        QUrl url("https://api.snapcraft.io/api/v1/snaps/download/oHNBJUD1kdfzgjkRHBz7kDGcyfgzbMD1_15.snap");
        QUrl url(info_map["url"]);

        QNetworkRequest request;
        request.setUrl(url);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

        qInfo() << url;
        QNetworkReply *reply = manager->get(request);

        QEventLoop eventLoop;
        connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();       //block until finish
        QString fileName = info_map["name"] + "_" + info_map["architecture"] + ".snap";

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qInfo() << "Failed to open file" << fileName;
            return;
        }
        QByteArray responseByte = reply->readAll();
        qInfo() << "httprequest finish!";
        file.write(responseByte);
        file.close();
        qInfo() << "httpwrite finish!";

        reply->deleteLater();
        manager->deleteLater();
}


