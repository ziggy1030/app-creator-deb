#ifndef HTTPSDOWNLOAD_H
#define HTTPSDOWNLOAD_H

#include <QObject>

class HttpsDownload : public QObject
{
public:
    HttpsDownload();

    void DownLoading(QMap<QString, QString> info_map);

};

#endif // HTTPSDOWNLOAD_H
