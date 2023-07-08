#ifndef QT_NETWORK_REQUEST_H
#define QT_NETWORK_REQUEST_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QMutex>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <future>

class QtNetworkRequest : public QObject {
    Q_OBJECT

public:
    QtNetworkRequest(QObject *parent = nullptr);
    ~QtNetworkRequest();

    QNetworkReply::NetworkError get(const std::string &url, std::string &response, const std::map<std::string, std::string> &headers);

    QNetworkReply::NetworkError post(const std::string &url, const QByteArray &data, std::string &response, const std::map<std::string, std::string> &headers);    QNetworkReply::NetworkError downloadFile(const std::string &url, const std::string &destination, int numChunks = 4);

    qint64 getFileSize(const std::string &url);
    uint64_t wgetFileSize(const std::string &url);
    std::future<qint64> agetFileSize(const std::string &url);

    void skipPeerVerification(bool skip = true);
    void skipHostVerification(bool skip = true);

    void setProgressCallback(std::function<void(double, double, qint64)> callback);
    void setTProgressCallback(std::function<void(double, double, qint64, int)> callback);


    static QtNetworkRequest *qhttp();

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onReadyRead();
    void onFinished();
    void onTimeout();

private:
    void setHeaders(const std::map<std::string, std::string> &headers);
    QNetworkRequest createRequest(const QString &url, const std::map<std::string, std::string> &headers);

    QNetworkAccessManager networkAccessManager;
    QNetworkReply *networkReply;
    QFile outputFile;

    struct ChunkProgress {
        double progress;
        double dlnow;
        double dltotal;
    };

    std::vector<ChunkProgress> chunkProgress;
    qint64 totalLength;
    QMutex progressMutex;

    std::function<void(double progress, double downloadSpeed, qint64 totalSize)> progressCallbackFunction;
    std::function<void(double progress, double downloadSpeed, qint64 totalSize, int index)> tprogressCallbackFunction;
};

#endif // QT_NETWORK_REQUEST_H
