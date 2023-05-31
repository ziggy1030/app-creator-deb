#ifndef BUILDDEB_H
#define BUILDDEB_H

#include <QObject>
#include <QMap>


class BuildDeb : public QObject
{
    Q_OBJECT
public:
    explicit BuildDeb(QObject *parent = nullptr);

    //初始化文件结构
    void InitDir();

    //打包
    void BuildPkg();
    void RecvBuildInfo(QMap<QString,QString> build_info);
//    void RecvOutputPath(QString path);
//    void RecvSrcPath(QString path);

signals:

public slots:

private:
    QString m_app_name;
    QString m_version;
    QString m_arch;
    QString m_pkg_name;
    QString m_output_path;
    QString m_src_path;
};

#endif // BUILDDEB_H
