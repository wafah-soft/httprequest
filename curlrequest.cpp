#include "curlrequest.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include "curl/inc/curlver.h"

#ifndef qtnetwork

CurlRequest::func_list CurlRequest::func = none_function;

CurlRequest::CurlRequest() {
    InitSession();
}

CurlRequest::~CurlRequest() {
    DestroySession();
}

void CurlRequest::InitSession()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    multiHandle = curl_multi_init();
    headerList = nullptr;
    progressCallbackFunction = nullptr;
    tprogressCallbackFunction = nullptr;
}

void CurlRequest::DestroySession()
{
    if (headerList) {
        curl_slist_free_all(headerList);
    }
    curl_easy_cleanup(curl);
    curl_multi_cleanup(multiHandle);
    curl_global_cleanup();
}

CURLcode CurlRequest::get(const std::string& url, std::string &response,
                          const std::map<std::string, std::string>& headers) {

    func = get_function;

    setHeaders(headers);
    if(!curl) return CURLE_FAILED_INIT;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    return performRequest();
}

CURLcode CurlRequest::post(const std::string& url, const std::string& data, std::string &response,
                           const std::map<std::string, std::string>& headers) {

    func = post_function;

    setHeaders(headers);

    if(!curl) return CURLE_FAILED_INIT;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    return performRequest();
}

CURLcode CurlRequest::downloadFile(const std::string &url, const std::string &destination, int numChunks)
{
    if (numChunks <= 0) {
        throw std::runtime_error("Invalid number of chunks specified.");
    }

    func = download_function;

    chunkProgress.resize(numChunks);
    progressDataList.resize(numChunks);


    // Get the file size
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_perform(curl);

    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    double contentLength = 0.0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);

    if (contentLength <= 0.0) {
       return CURLE_HTTP_RETURNED_ERROR;
    }
    // Calculate the size of each chunk
    size_t chunkSize = static_cast<size_t>(contentLength) / numChunks;
    size_t remainingBytes = static_cast<size_t>(contentLength) % numChunks;

    // Download each chunk
    std::vector<std::future<CURLcode>> chunkFutures;
    std::vector<std::vector<char>> chunkBuffers(numChunks);

    int chunkIndex = 0;
    for (int i = 0; i < numChunks; ++i) {
        size_t startRange = i * chunkSize;
        size_t endRange = (i + 1) * chunkSize - 1;

        if (i == numChunks - 1) {
            endRange += remainingBytes;
        }

        progressDataList[i].index = i;
        progressDataList[i].instance = this;
        progressDataList[i].chunkCurl = nullptr;

        std::string range = std::to_string(startRange) + "-" + std::to_string(endRange);

        chunkFutures.emplace_back(std::async(std::launch::async, &CurlRequest::downloadChunk, this, url, range, std::ref(chunkBuffers[i]), chunkIndex));
    }

    // Wait for all chunks to download
    CURLcode result = CURLE_OK;
    for (auto& chunkFuture : chunkFutures) {
       CURLcode chunkResult = chunkFuture.get();
       if (chunkResult != CURLE_OK) {
           result = chunkResult;
       }
    }

    if (result != CURLE_OK) {
       return result;
    }

    // Write chunks to the destination file
    std::ofstream destFile(destination, std::ios::binary);
    if (!destFile.is_open()) {
       return CURLE_WRITE_ERROR;
    }

    for (const auto& chunkBuffer : chunkBuffers) {
       destFile.write(chunkBuffer.data(), chunkBuffer.size());
    }

    destFile.close();

    return CURLE_OK;
}

uint64_t CurlRequest::getFileSize(const std::string &url)
{
    func = get_file_size;

    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        return -1.0;
    }

    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    double contentLength = 0.0;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);


    if (contentLength <= 0.0) {
       return CURLE_HTTP_RETURNED_ERROR;
    }


    return contentLength;
}

void CurlRequest::skipPeerVerification(bool skip) {
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, skip ? 0L : 1L);
    }
}

void CurlRequest::skipHostVerification(bool skip) {
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, skip ? 0L : 2L);
    }
}

void CurlRequest::setProgressCallback(void (__stdcall *callback) (double, double, uint64_t))
{
    tprogressCallbackFunction = callback;
}

void CurlRequest::setProgressCallback(void (__stdcall *callback) (double, double, uint64_t, int))
{
    progressCallbackFunction = callback;
}

CURLcode CurlRequest::downloadChunk(const std::string &url, const std::string &range, std::vector<char> &buffer, int chunkIndex) {
    CURL* chunkCurl = curl_easy_duphandle(curl);
    if (!chunkCurl) {
        return CURLE_FAILED_INIT;
    }

    size_t writePosition = 0;

    progressDataList[chunkIndex].chunkCurl = chunkCurl;

    std::pair<std::vector<char>&, size_t&> bufferAndPosition = std::make_pair(std::ref(buffer), std::ref(writePosition));

    curl_easy_setopt(chunkCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(chunkCurl, CURLOPT_RANGE, range.c_str());
    curl_easy_setopt(chunkCurl, CURLOPT_WRITEDATA, &bufferAndPosition);
    curl_easy_setopt(chunkCurl, CURLOPT_WRITEFUNCTION, writeMemoryCallback);
    curl_easy_setopt(chunkCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(chunkCurl, CURLOPT_PROGRESSDATA, &progressDataList[chunkIndex]);
    curl_easy_setopt(chunkCurl, CURLOPT_PROGRESSFUNCTION, progressCallback);
    curl_easy_setopt(chunkCurl, CURLOPT_PRIVATE, &chunkIndex); // Set chunkIndex as private data


    CURLcode result = curl_easy_perform(chunkCurl);
    curl_easy_cleanup(chunkCurl);

    return result;
}

size_t CurlRequest::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {

    std::string* response = static_cast<std::string*>(userp);

    if(contents == nullptr || userp == nullptr) return 0;

    QByteArray data = QByteArray((char*)contents, size * nmemb);

    if(data.isEmpty()) return 0;

    bool should_append_data = true;

    if (func == download_function) {
        should_append_data = false;
    } else if (func == get_file_size) {
        should_append_data = false;
    }

    if(should_append_data) {
        response->append(data.data(), data.size());
    }

    return size * nmemb;
}

size_t CurlRequest::fileWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::ofstream* fileStream = static_cast<std::ofstream*>(userdata);
    if (fileStream) {
        fileStream->write(static_cast<char*>(ptr), size * nmemb);
        return size * nmemb;
    }
    return 0;
}

size_t CurlRequest::progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    ProgressData* data = static_cast<ProgressData*>(clientp);
    CurlRequest* instance = data->instance;
    CURL* chunkCurl = data->chunkCurl;
    int chunkIndex = data->index;

    if (dltotal > 0.0 && instance->progressCallbackFunction) {

        {
            std::unique_lock<std::mutex> lock(instance->progressMutex);
            instance->chunkProgress[chunkIndex].dlnow = dlnow;
            instance->chunkProgress[chunkIndex].dltotal = dltotal;
            instance->chunkProgress[chunkIndex].progress = (dlnow / dltotal) * 100.0;
        }


        double downloadSpeed = 0;
        double totalProgress = 0.0;

        double totalDlNow = 0;
        double totalDlTotal = 0;

        {
            std::unique_lock<std::mutex> lock(instance->progressMutex);
            for (const auto& progress : instance->chunkProgress) {
                totalDlNow += progress.dlnow;
                totalDlTotal += progress.dltotal;
//                totalProgress += progress.progress;
            }
        }

        totalProgress = (totalDlNow / totalDlTotal) * 100.0;

        if (totalDlTotal > 0) {
            curl_easy_getinfo(chunkCurl, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
            if(instance->progressCallbackFunction != NULL) {
                instance->progressCallbackFunction(totalProgress, downloadSpeed, totalDlTotal, chunkIndex);
            }
        }
    }

    return 0;
}

size_t CurlRequest::TprogressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    CurlRequest* instance = static_cast<CurlRequest*>(clientp);

    if (dltotal > 0.0 && instance->progressCallbackFunction) {
        double progress = (dlnow / dltotal) * 100.0;
        double downloadSpeed;
        curl_easy_getinfo(instance->curl, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
        instance->tprogressCallbackFunction(progress, downloadSpeed, dltotal);
    }
    return 0;
}


void CurlRequest::setHeaders(const std::map<std::string, std::string>& headers) {
    if (headerList) {
        curl_slist_free_all(headerList);
        headerList = nullptr;
    }

    for (const auto& header : headers) {
        std::string headerLine = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerLine.c_str());
    }

    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }
}

CURLcode CurlRequest::performRequest() {
    if (!curl) return CURLE_FAILED_INIT;
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) return res;
    return CURLE_OK;
}

size_t CurlRequest::CurlRequest::writeMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {

    size_t realsize = size * nmemb;
    auto* bufferAndPosition = static_cast<std::pair<std::vector<char>&, size_t&>*>(userp);
    std::vector<char>& buffer = bufferAndPosition->first;
    size_t& writePosition = bufferAndPosition->second;

    if (buffer.size() < writePosition + realsize) {
        buffer.resize(writePosition + realsize);
    }

    memcpy(buffer.data() + writePosition, contents, realsize);
    writePosition += realsize;

    return realsize;
}

#endif
