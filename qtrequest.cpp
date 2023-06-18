#include "qtrequest.h"
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QDateTime>
#include <QDir>
#include <QSemaphore>
#include <QTimer>
#include <QCoreApplication>
#include <windows.h>
#include <winhttp.h>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

QtNetworkRequest::QtNetworkRequest(QObject *parent)
    : QObject(parent), networkReply(nullptr) {
    tprogressCallbackFunction = nullptr;
    progressCallbackFunction = nullptr;
}

QtNetworkRequest::~QtNetworkRequest() {
    if (networkReply) {
        networkReply->abort();
        networkReply->deleteLater();
    }
}

void QtNetworkRequest::initSession() {
}

void QtNetworkRequest::destroySession() {
}

QNetworkReply::NetworkError QtNetworkRequest::get(const std::string &url, std::string &response, const std::map<std::string, std::string> &headers) {
    QNetworkRequest request = createRequest(QString::fromStdString(url), headers);
    networkReply = networkAccessManager.get(request);

    QEventLoop loop;
    connect(networkReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    response = networkReply->readAll().toStdString();
    QNetworkReply::NetworkError error = networkReply->error();
    networkReply->deleteLater();
    networkReply = nullptr;


    return error;
}

QNetworkReply::NetworkError QtNetworkRequest::post(const std::string &url, const QByteArray &data, std::string &response, const std::map<std::string, std::string> &headers) {
    QNetworkRequest request = createRequest(QString::fromStdString(url), headers);
    networkReply = networkAccessManager.post(request, data);

    QEventLoop loop;
    connect(networkReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QNetworkReply::NetworkError error = networkReply->error();

    response = networkReply->readAll().toStdString();

    networkReply->deleteLater();
    networkReply = nullptr;


    return error;
}

QNetworkReply::NetworkError QtNetworkRequest::downloadFile(const std::string &url, const std::string &destination, int numChunks) {

    qint64 fileSize = getFileSize(url);
    if (fileSize <= 0) {
       return QNetworkReply::ContentNotFoundError;
    }

    QFile outputFile(QString::fromStdString(destination));
    if (!outputFile.open(QIODevice::WriteOnly)) {
       return QNetworkReply::ContentAccessDenied;
    }

    qint64 chunkSize = fileSize / numChunks;
    std::vector<std::unique_ptr<QNetworkAccessManager>> managers(numChunks);
    std::vector<QNetworkReply *> replies;
    std::atomic<int> completedChunks{0};

    QEventLoop loop;
    QMutex mutex;

    for (int i = 0; i < numChunks; ++i) {

        managers[i] = std::make_unique<QNetworkAccessManager>();

        QString rangeHeaderValue = QString("bytes=%1-%2").arg(i * chunkSize).arg((i == numChunks - 1) ? "" : QString::number((i + 1) * chunkSize - 1));
        QNetworkRequest request = createRequest(QString::fromStdString(url), {{"Range", rangeHeaderValue.toStdString()}});

        QNetworkReply *reply = managers[i]->get(request);
        replies.push_back(reply);

        connect(reply, &QNetworkReply::finished, this, [this, &completedChunks, &loop, &mutex, reply, &outputFile, chunkSize, i, numChunks]() {
            QByteArray data = reply->readAll();
            reply->deleteLater();

            {
               QMutexLocker locker(&mutex);
               outputFile.seek(i * chunkSize);
               outputFile.write(data);
            }

            if (++completedChunks == numChunks) {
               loop.quit();
            }
        });


        connect(reply, &QNetworkReply::downloadProgress, this, [this, &mutex, chunkSize, fileSize](qint64 bytesReceived, qint64) {
            double progress = static_cast<double>(bytesReceived) / chunkSize * 100.0;
            mutex.lock();
            if (tprogressCallbackFunction) {
                tprogressCallbackFunction(progress, 0, fileSize, 0);
            }
            if (progressCallbackFunction) {
                progressCallbackFunction(progress, 0, fileSize);
            }
            mutex.unlock();
        });

        connect(reply, &QNetworkReply::sslErrors, this, [](const QList<QSslError> &errors) {
            for (const auto &error : errors) {
               throw error.errorString();
            }
        });
    }

    loop.exec();

    outputFile.close();

    QNetworkReply::NetworkError result = QNetworkReply::NoError;
    for (const auto &reply : replies) {
        if(reply) {
            if (reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::ContentOperationNotPermittedError) {
                result = reply->error();
                break;
            }
        }
    }

    return result;
}

qint64 QtNetworkRequest::getFileSize(const std::string &url) {
    QNetworkRequest request = createRequest(QString::fromStdString(url), {});
    networkReply = networkAccessManager.head(request);

    QEventLoop loop;
    connect(networkReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    qint64 size = networkReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    networkReply->deleteLater();
    networkReply = nullptr;
    return size;
}

uint64_t QtNetworkRequest::wgetFileSize(const std::string &url)
{

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    DWORD dwFileSize = 0;

    URL_COMPONENTS urlComponents;
    wchar_t hostName[256];
    wchar_t urlPath[1024];
    ZeroMemory(&urlComponents, sizeof(urlComponents));
    ZeroMemory(hostName, sizeof(hostName));
    ZeroMemory(urlPath, sizeof(urlPath));


    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.lpszHostName = hostName;
    urlComponents.dwHostNameLength = sizeof(hostName) / sizeof(hostName[0]);
    urlComponents.lpszUrlPath = urlPath;
    urlComponents.dwUrlPathLength = sizeof(urlPath) / sizeof(urlPath[0]);
    urlComponents.nScheme = INTERNET_SCHEME_HTTPS;

    std::wstring wideUrl(url.begin(), url.end());

    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &urlComponents)) {
        std::cerr << "Error: Invalid URL" << std::endl;
        return 0;
    }

    hSession = WinHttpOpen(L"TFMToolPro/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::cerr << "Error: WinHttpOpen failed" << std::endl;
        return 0;
    }

    hConnect = WinHttpConnect(hSession, urlComponents.lpszHostName, urlComponents.nPort, 0);
    if (!hConnect) {
        std::cerr << "Error: WinHttpConnect failed" << std::endl;
        WinHttpCloseHandle(hSession);
        return 0;
    }

    hRequest = WinHttpOpenRequest(hConnect, L"HEAD", urlComponents.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        std::cerr << "Error: WinHttpOpenRequest failed" << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        std::cerr << "Error: WinHttpSendRequest failed" << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        std::cerr << "Error: WinHttpReceiveResponse failed" << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    DWORD dwSize = sizeof(dwFileSize);
    if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwFileSize, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
        std::cerr << "Error: WinHttpQueryHeaders failed" << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 0;
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return static_cast<uint64_t>(dwFileSize);
}

std::future<qint64> QtNetworkRequest::agetFileSize(const std::string &url)
{
    QNetworkRequest request = createRequest(QString::fromStdString(url), {});
    networkReply = networkAccessManager.head(request);

    auto promise = std::make_shared<std::promise<qint64>>();

    connect(networkReply, &QNetworkReply::finished, [this, promise]() {
        qint64 size = networkReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        networkReply->deleteLater();
        networkReply = nullptr;
        promise->set_value(size);
    });

    return promise->get_future();
}

void QtNetworkRequest::skipPeerVerification(bool skip)
{
    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    sslConfiguration.setPeerVerifyMode(skip ? QSslSocket::VerifyNone : QSslSocket::AutoVerifyPeer);
    QSslConfiguration::setDefaultConfiguration(sslConfiguration);
}

void QtNetworkRequest::skipHostVerification(bool skip)
{
    QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
    QSslConfiguration::setDefaultConfiguration(sslConfiguration);
}

void QtNetworkRequest::setProgressCallback(std::function<void (double, double, qint64)> callback)
{
    progressCallbackFunction = callback;
}

void QtNetworkRequest::setTProgressCallback(std::function<void (double, double, qint64, int)> callback)
{
    tprogressCallbackFunction = callback;
}
void QtNetworkRequest::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    int chunkIndex = reply->property("chunkIndex").toInt();
    double progress = (double)bytesReceived / bytesTotal;

    if (tprogressCallbackFunction) {
        qint64 totalReceived = 0;
        for (int i = 0; i < chunkProgress.size(); i++) {
            totalReceived += chunkProgress[i].dlnow;
        }
        double totalProgress = (double)totalReceived / totalLength;
        double downloadSpeed = reply->attribute(QNetworkRequest::DownloadBufferAttribute).toDouble();
        tprogressCallbackFunction(totalProgress, downloadSpeed, totalLength, chunkIndex);
    } else if (progressCallbackFunction) {
        double downloadSpeed = reply->attribute(QNetworkRequest::DownloadBufferAttribute).toDouble();
        progressCallbackFunction(progress, downloadSpeed, bytesTotal);
    }
}

void QtNetworkRequest::onReadyRead()
{
    outputFile.write(networkReply->readAll());
}

void QtNetworkRequest::onFinished()
{
    outputFile.write(networkReply->readAll());
}

void QtNetworkRequest::onTimeout()
{
    if(networkReply) {
        networkReply->abort();
    }
}

void QtNetworkRequest::setHeaders(const std::map<std::string, std::string> &headers)
{
    for (const auto &header : headers) {
        QString key = QString::fromStdString(header.first);
        QString value = QString::fromStdString(header.second);
        networkReply->request().setRawHeader(key.toUtf8(), value.toUtf8());
    }
}

QNetworkRequest QtNetworkRequest::createRequest(const QString &url, const std::map<std::string, std::string> &headers) {

    QNetworkRequest request;
    request.setUrl(QUrl(url));

    for (const auto &header : headers) {
        request.setRawHeader(QByteArray::fromStdString(header.first), QByteArray::fromStdString(header.second));
    }

    // Set SSL configuration to skip peer and host verification if needed
    QSslConfiguration sslConfiguration = request.sslConfiguration();
    sslConfiguration.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);
    request.setSslConfiguration(sslConfiguration);

    return request;
}

