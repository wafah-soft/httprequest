#ifndef FILESIZEGETTER_H
#define FILESIZEGETTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSemaphore>

class FileSizeGetter : public QObject
{
    Q_OBJECT
public:
    explicit FileSizeGetter(QObject *parent = nullptr);
    qint64 getFileSize(const QString &url);

private slots:
    void onNetworkReplyFinished();

private:
    QNetworkAccessManager networkAccessManager;
    QNetworkReply *networkReply;
    QSemaphore semaphore;
    qint64 fileSize;
};
#endif // FILESIZEGETTER_H
