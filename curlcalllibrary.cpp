#include "curlcalllibrary.h"

#ifndef qtnetwork

CurlCallLibrary *CurlCallLibrary::instance = NULL;

CurlCallLibrary::CurlCallLibrary()
{
    instance = this;
    curlRequest = std::shared_ptr<CurlRequest>(new CurlRequest);
    curlRequest->skipHostVerification();
    curlRequest->skipPeerVerification();
}

CurlCallLibrary::~CurlCallLibrary()
{
//    curlRequest.reset();
}

CURLcode CurlCallLibrary::send_post_request(QString url, wafah_data *data)
{
    return instance->curlRequest->post(url.toStdString(), data->post_data.toStdString(), data->response, data->header_list);
}

CURLcode CurlCallLibrary::send_get_request(QString url, wafah_data *data)
{
    return instance->curlRequest->get(url.toStdString(), data->response, data->header_list);
}

CURLcode CurlCallLibrary::https_download_file(wafah_download_arg *arg)
{
    if(arg->pcallback != NULL)
        instance->curlRequest->setProgressCallback(arg->pcallback);

    if(arg->chunkProgressCallback != NULL)
        instance->curlRequest->setProgressCallback(arg->chunkProgressCallback);

    return instance->curlRequest->downloadFile(arg->url.toStdString(), arg->file_name.toStdString());
}

quint64 CurlCallLibrary::get_remote_file_size(QString url)
{
    return instance->curlRequest->getFileSize(url.toStdString());
}

#endif
