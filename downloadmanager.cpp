#include "downloadmanager.h"
#include <QDir>
#include <QDebug>

DownloadManager::DownloadManager()
{

}

void DownloadManager::do_download(const QUrl &url)
{
    QEventLoop loop;

    connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(download_finished(QNetworkReply*)));

    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            SLOT(sslErrors(QList<QSslError>)));
#endif

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));

    loop.exec();

    downloadTimer.start();

    currentDownloads.append(reply);

//    download_finished(reply);
}

QString DownloadManager::save_file_name(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if(basename.isEmpty())
        basename = "download";

    if(QFile::exists(basename)) {
        int i = 0;
        basename += '.';
        while(QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

bool DownloadManager::save_to_disk(const QString &filename, QIODevice *data)
{
    //QDir::toNativeSeparators(QDir::currentPath() + "/" + filename);

    QFile file(output_filename);
    if(!file.open(QIODevice::WriteOnly)) {
        qInfo() << "Failed to open file...";
        return (false);
    }

    file.write(data->readAll());
    file.close();

    return (true);
}

void DownloadManager::execute()
{

}

void DownloadManager::download_finished(QNetworkReply *reply)
{
    auto url = reply->url();
    if(reply->error()) {
        fprintf(stderr, "Download of %s failed: %s\n",
                       url.toEncoded().constData(),
                       qPrintable(reply->errorString()));
    }
    else {
        QString filename = save_file_name(url);
        qInfo() << filename;
        if(save_to_disk(filename, reply)) {
            qInfo("Download of %s succeeded (saved to %s)\n",
                               url.toEncoded().constData(), qPrintable(filename));
        }
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();
}

void DownloadManager::sslErrors(const QList<QSslError> &errors)
{

}

void DownloadManager::downloadProgress(qint64 downloaded, qint64 total)
{

    double percent = (downloaded * 100.0) / total;

    double speed = downloaded * 1000.0 / downloadTimer.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    dl_progress(percent, QString::fromLatin1("%1 %2")
                .arg(speed, 3, 'f', 1).arg(unit), total);
}

void DownloadManager::setOutput_filename(const QString &value)
{
    output_filename = value;
}

void DownloadManager::set_download_callback(const download_progress &value)
{
    dl_progress = value;
}
