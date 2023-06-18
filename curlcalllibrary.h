#ifndef CURLCALLLIBRARY_H
#define CURLCALLLIBRARY_H

#include "httpclientdef.h"
#include "curlrequest.h"

#ifndef qtnetwork

class CurlCallLibrary
{
public:
    CurlCallLibrary();
    ~CurlCallLibrary();

    static CURLcode send_post_request(QString url, wafah_data *data);
    static CURLcode send_get_request(QString url, wafah_data *data);
    static CURLcode https_download_file(wafah_download_arg *arg);
    static quint64 get_remote_file_size(QString url);

private:
    std::shared_ptr<CurlRequest> curlRequest;
    static CurlCallLibrary *instance;
};

#endif

#endif // CURLCALLLIBRARY_H
