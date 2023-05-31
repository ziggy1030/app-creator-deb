#ifndef EXTRACTDEB_H
#define EXTRACTDEB_H

#include <QObject>

class DebHandle : public QObject
{
    Q_OBJECT
public:
    explicit DebHandle(QObject *parent = nullptr);

    //解压deb包
    void ExtractDeb(QString path);

    //读取包内信息
    void ReadDebianInfo();


    QString CallCMD(QString commands);

    QString m_file_path;

};

#endif // DEBHANDLE_H
