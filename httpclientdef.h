#ifndef HTTPCLIENTDEF_H
#define HTTPCLIENTDEF_H

#include <QObject>
#include <QString>
#include "curl/inc/curl.h"

typedef void(__stdcall *download_progress)(int percentage, QString speed, qint64 total);
typedef void(__stdcall *progressCallbackFunction)(double progress, double downloadSpeed, uint64_t totalSize);
typedef void(__stdcall *chprogressCallbackFunction)(double progress, double downloadSpeed, uint64_t totalSize, int index);


typedef void(__stdcall *wafah_https_progress)(quint64 dltotal, quint64 dlnow,
                                              quint64 ultotal, quint64 ulnow);

typedef struct {
    curl_slist *cookies = NULL;
    long http_status;
    QString content_type;
    QString header_data;
} wafah_header_info;

typedef struct {
    QString post_data;
    std::map<std::string, std::string> header_list;
    QString cookie;
    std::string response;
    wafah_header_info header_info;
} wafah_data;

typedef struct {
    CURL *curl;
    struct curl_slist *chunk = NULL;
} wafah_handler;

typedef struct {
    wafah_https_progress progress_calback;
} wafah_download_cb;

typedef struct {
    QString save_file_name;
    QString headers;
    QString cookie;
    QString url;
    quint64 from_range;
    wafah_download_cb callback;
    wafah_header_info header_info;
} wafah_download;

typedef struct {
    QString url;
    quint64 range = 0;
    QString file_name;
    QString header;
    int http_status;
    download_progress callback;
    progressCallbackFunction pcallback;
    chprogressCallbackFunction chunkProgressCallback;
} wafah_download_arg;


#endif // HTTPCLIENTDEF_H
