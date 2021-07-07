#include "httpclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>


static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)dest, size * nmemb);
    return size * nmemb;
}

static size_t header_callback(char *dest, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)dest, size * nmemb);
    return size *nmemb;
}

int http_post(wafah_handler *handler, std::string url, wafah_data *data)
{

    std::string resp, header_data, post_data;

    post_data = data->post_data.toStdString().c_str();

    char *c_info;
    long res_code;

    CURLcode res = CURLE_OK;

    curl_global_init(CURL_GLOBAL_ALL);

    auto curl = handler->curl;

    curl = curl_easy_init();
    if(curl) {

        if(handler->chunk) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, handler->chunk);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_POST, true);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);

        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_data);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

        res = curl_easy_perform(curl);

        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &c_info);
        data->header_info.content_type = c_info;

        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
        data->header_info.http_status = res_code;

        //set cookie list
        res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, data->header_info.cookies);

        if(handler->chunk) {
            curl_slist_free_all(handler->chunk);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    *data->response = resp;
    data->header_info.header_data = header_data;

    return res;
}
