#ifndef CURL_REQUEST_H
#define CURL_REQUEST_H

#include <QByteArray>
#include <string>
#include <map>
#include <curl/inc/curl.h>
#include <vector>
#include <future>
#include <memory>
#include <atomic>


#ifndef qtnetwork

class CurlRequest {
public:
    CurlRequest();
    ~CurlRequest();

    void InitSession();

    void DestroySession();

    CURLcode get(const std::string& url, std::string &response, const std::map<std::string, std::string>& headers = {});
    CURLcode post(const std::string& url, const std::string& data, std::string &response, const std::map<std::string, std::string>& headers = {});
    CURLcode downloadFile(const std::string& url, const std::string& destination, int numChunks = 4);
    uint64_t getFileSize(const std::string& url);

    void skipPeerVerification(bool skip = true);
    void skipHostVerification(bool skip = true);

    void setProgressCallback(void (__stdcall *callback) (double, double, uint64_t));
    void setProgressCallback(void (__stdcall *callback) (double, double, uint64_t, int));


    CURLcode downloadChunk(const std::string& url, const std::string& range, std::vector<char>& buffer, int chunkIndex);


private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t fileWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
    static size_t progressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static size_t TprogressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);


    void setHeaders(const std::map<std::string, std::string>& headers);
    CURLcode performRequest();
    struct curl_slist* headerList;
    void (__stdcall *progressCallbackFunction)(double progress, double downloadSpeed, uint64_t totalSize, int index);
    void (__stdcall *tprogressCallbackFunction)(double progress, double downloadSpeed, uint64_t totalSize);


    static size_t writeMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp);

private:
    CURL* curl;
    CURLM* multiHandle;

    struct ChunkProgress {
        double progress;
        double dlnow;
        double dltotal;
    };

    struct ProgressData {
        CurlRequest* instance;
        CURL* chunkCurl;
        int index;
    };

    std::vector<ChunkProgress> chunkProgress;
    std::vector<ProgressData> progressDataList;
    uint64_t totalLength;

    std::mutex progressMutex;


    typedef enum function_list {
        none_function,
        get_function,
        post_function,
        download_function,
        get_file_size
    } func_list;

    static func_list func;
};

#endif

#endif // CURL_REQUEST_H
