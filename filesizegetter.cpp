#include "filesizegetter.h"

#include <QDebug>

FileSizeGetter::FileSizeGetter(QObject *parent) : QObject(parent)
{
    networkReply = nullptr;
}

qint64 FileSizeGetter::getFileSize(const QString &url)
{
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    networkReply = networkAccessManager.head(request);

    connect(networkReply, &QNetworkReply::finished, this, &FileSizeGetter::onNetworkReplyFinished);

    // Wait for the finished signal
    semaphore.acquire(1);

    // Process the result and return the file size
    if (networkReply->error() == QNetworkReply::NoError)
    {
        fileSize = networkReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    }
    else
    {
        fileSize = 0;
        qDebug() << "Error:" << networkReply->errorString();
    }

    networkReply->deleteLater();
    networkReply = nullptr;

    return fileSize;
}

void FileSizeGetter::onNetworkReplyFinished()
{
    semaphore.release(1);
}
