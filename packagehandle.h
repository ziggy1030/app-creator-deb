/*
包括snap包的json信息解析、下载、解压
*/
#ifndef PACKAGEHANDLE_H
#define PACKAGEHANDLE_H
#include <QString>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QDir>


class PackageHandle : public QObject
{
    Q_OBJECT
public:
    explicit PackageHandle(QObject *parent = nullptr);

    //请求json数据
    QByteArray RequestSnap(QString app_name);

    //传入请求到的snap json信息，根据传入的arch过滤出合适的应用名、包名、版本号、架构等
    QMap<QString, QString> FilteringInfo(QByteArray snap_info,QString arch);

    //下载snap包
    void DownloadSnap(QString snap_url, QString name, QString architecture);

    //解压snap包
    void UnsquashfsSnap(QString snap_pkg);

    void DebHand(QString path);
    void SnapHand(QString app_name,QString arch);

    //解压deb包
    void ExtractDeb(QString path);

    /**
     * @brief ReadDebian
     * @param path QDir::currentPath()+"/snapsrcfile"
     * @param filename control/desktop/info
     */
    void ReadDebian(QString path);
    void ReadControlFile(QString path);
    void ReadDesktopFile(QString path);
    void ReadInfoFile(QString path);




    QString CallCMD(QString commands);
    QString SearchFile(QString path, QString filename);
    QByteArray ReadText(QString path);
    void Write2File(QString filepath_, QString text_);


    //打包
    void BuildDebPkg(QMap<QString,QString> map_);

    //初始化新的包目录
    void InitNewDir(QString path);


private:
    QString m_filename;
    QString m_deb_src_path;
    QString m_snap_src_path;
    QString m_desktop_name;
    QString m_appname;
    QString m_pkgname;
    QString m_version;
    QString m_arch;
    QString m_homepage;
    QString m_description;
    QString m_iconname;
    QString m_icon_abpath;
    QString m_execPath;
    QMap<QString,QString> m_map;





signals:
    void sendSnapInfo(QMap<QString, QString> info_map);
    void responseErr();
    void typeErr(QString type);
    void sendControl(QByteArray bytes);
    void sendDesktop(QString filename, QByteArray bytes);
    void sendUOSInfo(QByteArray bytes);

    void BuildFinish();


public slots:




};

#endif // PACKAGEHANDLE_H
