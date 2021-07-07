#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslError>
#include <QStringList>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QUrl>
#include <stdio.h>
#include "httpclientdef.h"


class DownloadManager : public QObject
{
    Q_OBJECT
public:
    DownloadManager();
    void do_download(const QUrl &url);
    QString save_file_name(const QUrl &url);
    bool save_to_disk(const QString &filename, QIODevice *data);

    void set_download_callback(const download_progress &value);

    void setOutput_filename(const QString &value);

    void send_post(QString url, wafah_data *data);

public slots:
    void execute();
    void download_finished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
    void downloadProgress(qint64 downloaded, qint64 total);


private:
    QNetworkAccessManager manager;
    QList<QNetworkReply *> currentDownloads;
    download_progress dl_progress;
    QElapsedTimer downloadTimer;
    QString output_filename;

};

#endif // DOWNLOADMANAGER_H
