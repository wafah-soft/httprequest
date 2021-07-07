#ifndef HTTPCLIENTDEF_H
#define HTTPCLIENTDEF_H

#include <QObject>
#include "curl/inc/curl.h"

typedef void(__stdcall *download_progress)(int percentage, QString speed, qint64 total);

typedef struct {
    curl_slist *cookies = NULL;
    long http_status;
    std::string content_type;
    std::string header_data;
} wafah_header_info;


typedef struct {
    QString post_data;
    QString headers;
    std::string *response;
    wafah_header_info header_info;
} wafah_data;

typedef struct {
    CURL *curl;
    struct curl_slist *chunk = NULL;
} wafah_handler;


#endif // HTTPCLIENTDEF_H
