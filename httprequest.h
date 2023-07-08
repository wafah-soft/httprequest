#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


#include <QString>
#include "httpclientdef.h"
#include <QNetworkReply>
#include <QtCore>

#ifndef qtnetwork

QString get_file_name_from_url(QString url);

CURLcode send_post_request(QString url, wafah_data *data);

CURLcode send_get_request(QString url, wafah_data *data);

CURLcode https_download_file(wafah_download_arg *arg);

quint64 get_remote_file_size(QString url);

#else

QNetworkReply::NetworkError qsend_post_request(const QString &url, wafah_data *data);

QNetworkReply::NetworkError qsend_get_request(const QString &url, wafah_data *data);

qint64 qget_remote_file_size(const QString &url);

QNetworkReply::NetworkError qhttps_download_file(wafah_download_arg *arg);

#endif

const char *qcurl_easy_strerror(int error);

#endif // HTTPREQUEST_H
