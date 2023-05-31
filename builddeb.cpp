#include "builddeb.h"
#include <QDebug>


BuildDeb::BuildDeb(QObject *parent) : QObject(parent)
{

}

void BuildDeb::RecvBuildInfo(QMap<QString, QString> build_info)
{
    m_app_name = build_info["app_name"];
    m_version = build_info["version"];
    m_arch = build_info["arch"];
    m_pkg_name = build_info["pkg_name"];
    qInfo() <<  __FUNCTION__ << "m_app_name" << m_app_name;
    qInfo() <<  __FUNCTION__ << "m_version" << m_version;
    qInfo() <<  __FUNCTION__ << "m_arch" << m_arch;
    qInfo() <<  __FUNCTION__ << "m_pkg_name" << m_pkg_name;

}

//void BuildDeb::RecvOutputPath(QString path)
//{
//    m_output_path = path;
//    qInfo() << __FUNCTION__ << "m_output_path" << m_output_path;
//}

//void BuildDeb::RecvSrcPath(QString path)
//{
//    m_src_path = path;
//    qInfo() <<  __FUNCTION__ << "m_output_path" << m_output_path;

//}
