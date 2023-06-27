#include "debhandle.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDirIterator>

DebHandle::DebHandle(QObject *parent) : QObject(parent)
  ,m_file_path("debsrcfile")
{

}

void DebHandle::ExtractDeb(QString path)
{
    qInfo() << "Extracting deb file...";
    const QString cmdstr_ = "dpkg-deb -R " + path + " debsrcfile";
    CallCMD(cmdstr_);
    qInfo() << m_file_path;
    qInfo() << "Extractingdeb over";


    ReadDebianInfo();


}

void DebHandle::ReadDebianInfo()
{
    QDir dir;
    QStringList filters;
    filters << "DEBIAN"
            << "debian";//过滤条件，可以添加多个选项，可以匹配文件后缀等。我这里只是找指定文件

    dir.setPath(m_file_path);
    dir.setNameFilters(filters);//添加过滤器

    //QDirIterator 此类可以很容易的返回指定目录的所有文件及文件夹，可以再递归遍历，也可以自动查找指定的文件
    QDirIterator iter(dir,QDirIterator::Subdirectories);

    while (iter.hasNext())
    {
        iter.next();
        QFileInfo info=iter.fileInfo();
        if (info.isFile())
        {
            m_file_path.append(info.absoluteFilePath());
        }
    }

    QFile file(m_file_path);
    QByteArray bytes;
    if(!file.exists()) //文件不存在则退出
       {
           qDebug()<<"file not exist";
           return;

       }
       if(file.open(QFile::ReadOnly))
       {
           bytes = file.readAll();
//           qInfo() << bytes;
       }

       file.close();

}

QString DebHandle::CallCMD(QString commands)
{
    qInfo() << "Running:" << commands;
    QProcess process;
    process.start("bash", QStringList() << "-c" << commands);
    process.waitForFinished();
    qInfo() << "CallCMD Over!";
    return process.readAllStandardOutput();
}
