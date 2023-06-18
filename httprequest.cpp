#include "httprequest.h"
#include <QUrl>
#include <QFuture>
#include "curlrequest.h"
#include "curlcalllibrary.h"
#include "qtrequest.h"
#include "filesizegetter.h"
#include <QtConcurrent>

#ifndef qtnetwork
//CURLcode send_post_request(QString url, wafah_data *data)
//{
//    CurlRequest request;
//    request.skipHostVerification();
//    request.skipPeerVerification();
//    auto ret = request.post(url.toStdString(), data->post_data.toStdString(), data->response, data->header_list);
//    return ret;
//}

//CURLcode send_get_request(QString url, wafah_data *data)
//{
//    CurlRequest request;
//    request.skipHostVerification();
//    request.skipPeerVerification();
//    auto ret = request.get(url.toStdString(), data->response, data->header_list);
//    return ret;
//}

//CURLcode https_download_file(wafah_download_arg *arg)
//{
//    CurlRequest request;
//    request.setProgressCallback(arg->pcallback);
//    request.setProgressCallback(arg->chunkProgressCallback);
//    request.skipHostVerification();
//    request.skipPeerVerification();
//    auto ret = request.downloadFile(arg->url.toStdString(), arg->file_name.toStdString());
//    return ret;
//}

//quint64 get_remote_file_size(QString url)
//{
//    CurlRequest request;
//    request.skipHostVerification();
//    request.skipPeerVerification();
//    uint64_t ret = request.getFileSize(url.toStdString());
//    return ret;
//}

#else

std::future<QNetworkReply::NetworkError> qsend_post_request(const QString &url, wafah_data *data) {
    QtNetworkRequest request;
    QByteArray postData = data->post_data.toLocal8Bit(); // Assuming you have a method to convert wafah_data to QByteArray
    auto result = request.post(url.toStdString(), postData, data->response, data->header_list);
    return result;
}

std::future<QNetworkReply::NetworkError> qsend_get_request(const QString &url, wafah_data *data) {
    QtNetworkRequest request;
    auto result = request.get(url.toStdString(), data->response, data->header_list);
    return result;
}

std::future<qint64> qget_remote_file_size(const QString &url) {
    QtNetworkRequest request;
    auto result = request.agetFileSize(url.toStdString());
    return result;
}

QNetworkReply::NetworkError qhttps_download_file(wafah_download_arg *arg) {
    QtNetworkRequest request;
    request.setProgressCallback(arg->pcallback);
    request.setTProgressCallback(arg->chunkProgressCallback);
    return request.downloadFile(arg->url.toStdString(), arg->file_name.toStdString(), 4);
}

#endif

const char *qcurl_easy_strerror(int error) {
    switch (error) {
        case QNetworkReply::NoError: return "NoError";
        case QNetworkReply::ConnectionRefusedError: return "ConnectionRefusedError";
        case QNetworkReply::RemoteHostClosedError: return "RemoteHostClosedError";
        case QNetworkReply::HostNotFoundError: return "HostNotFoundError";
        case QNetworkReply::TimeoutError: return "TimeoutError";
        case QNetworkReply::OperationCanceledError: return "OperationCanceledError";
        case QNetworkReply::SslHandshakeFailedError: return "SslHandshakeFailedError";
        case QNetworkReply::TemporaryNetworkFailureError: return "TemporaryNetworkFailureError";
        case QNetworkReply::NetworkSessionFailedError: return "NetworkSessionFailedError";
        case QNetworkReply::BackgroundRequestNotAllowedError: return "BackgroundRequestNotAllowedError";
        case QNetworkReply::TooManyRedirectsError: return "TooManyRedirectsError";
        case QNetworkReply::InsecureRedirectError: return "InsecureRedirectError";
        case QNetworkReply::UnknownNetworkError: return "UnknownNetworkError";
        case QNetworkReply::ProxyConnectionRefusedError: return "ProxyConnectionRefusedError";
        case QNetworkReply::ProxyConnectionClosedError: return "ProxyConnectionClosedError";
        case QNetworkReply::ProxyNotFoundError: return "ProxyNotFoundError";
        case QNetworkReply::ProxyTimeoutError: return "ProxyTimeoutError";
        case QNetworkReply::ProxyAuthenticationRequiredError: return "ProxyAuthenticationRequiredError";
        case QNetworkReply::UnknownProxyError: return "UnknownProxyError";
        case QNetworkReply::ContentAccessDenied: return "ContentAccessDenied";
        case QNetworkReply::ContentOperationNotPermittedError: return "ContentOperationNotPermittedError";
        case QNetworkReply::ContentNotFoundError: return "ContentNotFoundError";
        case QNetworkReply::AuthenticationRequiredError: return "AuthenticationRequiredError";
        case QNetworkReply::ContentReSendError: return "ContentReSendError";
        case QNetworkReply::ContentConflictError: return "ContentConflictError";
        case QNetworkReply::ContentGoneError: return "ContentGoneError";
        case QNetworkReply::UnknownContentError: return "UnknownContentError";
        case QNetworkReply::ProtocolUnknownError: return "ProtocolUnknownError";
        case QNetworkReply::ProtocolInvalidOperationError: return "ProtocolInvalidOperationError";
        case QNetworkReply::ProtocolFailure: return "ProtocolFailure";
        case QNetworkReply::InternalServerError: return "InternalServerError";
        case QNetworkReply::OperationNotImplementedError: return "OperationNotImplementedError";
        case QNetworkReply::ServiceUnavailableError: return "ServiceUnavailableError";
        case QNetworkReply::UnknownServerError: return "UnknownServerError";
        default: return "UnknownError";
    }
//    switch (error) {
//       case CURLE_OK:
//           return "No error";
//       case CURLE_UNSUPPORTED_PROTOCOL:
//           return "Unsupported protocol";
//       case CURLE_FAILED_INIT:
//           return "Failed initialization";
//       case CURLE_URL_MALFORMAT:
//           return "URL is not correctly formatted";
//       case CURLE_NOT_BUILT_IN:
//           return "Requested feature/function not available in this version";
//       case CURLE_COULDNT_RESOLVE_PROXY:
//           return "Couldn't resolve proxy";
//       case CURLE_COULDNT_RESOLVE_HOST:
//           return "Couldn't resolve host";
//       case CURLE_COULDNT_CONNECT:
//           return "Couldn't connect";
//       case CURLE_WEIRD_SERVER_REPLY:
//           return "Weird server reply";
//       case CURLE_REMOTE_ACCESS_DENIED:
//           return "Remote access denied";
//       case CURLE_FTP_ACCEPT_FAILED:
//           return "FTP accept failed";
//       case CURLE_FTP_WEIRD_PASS_REPLY:
//           return "FTP weird PASS reply";
//       case CURLE_FTP_ACCEPT_TIMEOUT:
//           return "FTP accept timeout";
//       case CURLE_FTP_WEIRD_PASV_REPLY:
//           return "FTP weird PASV reply";
//       case CURLE_FTP_WEIRD_227_FORMAT:
//           return "FTP weird 227 format";
//       case CURLE_FTP_CANT_GET_HOST:
//           return "FTP can't get host";
//       case CURLE_HTTP2:
//           return "Error in the HTTP2 framing layer";
//       case CURLE_FTP_COULDNT_SET_TYPE:
//           return "FTP couldn't set file type";
//       case CURLE_PARTIAL_FILE:
//           return "Partial file";
//       case CURLE_FTP_COULDNT_RETR_FILE:
//           return "FTP couldn't retrieve file";
//       case CURLE_QUOTE_ERROR:
//           return "Quote error";
//       case CURLE_HTTP_RETURNED_ERROR:
//           return "HTTP returned error";
//       case CURLE_WRITE_ERROR:
//           return "Write error";
//       case CURLE_UPLOAD_FAILED:
//           return "Upload failed";
//       case CURLE_READ_ERROR:
//           return "Read error";
//       case CURLE_OUT_OF_MEMORY:
//           return "Out of memory";
//       case CURLE_OPERATION_TIMEDOUT:
//           return "Operation timed out";
//       case CURLE_FTP_PORT_FAILED:
//           return "FTP PORT operation failed";
//       case CURLE_FTP_COULDNT_USE_REST:
//           return "FTP REST command returned error";
//       case CURLE_RANGE_ERROR:
//           return "Range error";
//       case CURLE_HTTP_POST_ERROR:
//           return "HTTP POST error";
//       case CURLE_SSL_CONNECT_ERROR:
//           return "SSL connect error";
//       case CURLE_BAD_DOWNLOAD_RESUME:
//           return "Bad download resume";
//       case CURLE_FILE_COULDNT_READ_FILE:
//           return "File couldn't read file";
//       case CURLE_LDAP_CANNOT_BIND:
//           return "LDAP cannot bind";
//       case CURLE_LDAP_SEARCH_FAILED:
//           return "LDAP search failed";
//       case CURLE_FUNCTION_NOT_FOUND:
//           return "Function not found";
//       case CURLE_ABORTED_BY_CALLBACK:
//           return "Aborted by callback";
//       case CURLE_BAD_FUNCTION_ARGUMENT:
//           return "Bad function argument";
//       case CURLE_INTERFACE_FAILED:
//           return "Interface failed";
//       case CURLE_TOO_MANY_REDIRECTS:
//           return "Too many redirects";
//       case CURLE_UNKNOWN_OPTION:
//           return "Unknown option";
//        case CURLE_TELNET_OPTION_SYNTAX:
//            return "Telnet option syntax error";
//        case CURLE_PEER_FAILED_VERIFICATION:
//            return "Peer's certificate or SSH MD5 fingerprint was not verified";
//        case CURLE_GOT_NOTHING:
//            return "Got nothing";
//        case CURLE_SSL_ENGINE_NOTFOUND:
//            return "SSL crypto engine not found";
//        case CURLE_SSL_ENGINE_SETFAILED:
//            return "SSL crypto engine set failed";
//        case CURLE_SEND_ERROR:
//            return "Send error";
//        case CURLE_RECV_ERROR:
//            return "Receive error";
//        case CURLE_SSL_CERTPROBLEM:
//            return "Problem with the local client certificate";
//        case CURLE_SSL_CIPHER:
//            return "Couldn't use the specified cipher";
//        case CURLE_BAD_CONTENT_ENCODING:
//            return "Unrecognized or unsupported content encoding";
//        case CURLE_LDAP_INVALID_URL:
//            return "Invalid LDAP URL";
//        case CURLE_FILESIZE_EXCEEDED:
//            return "Maximum file size exceeded";
//        case CURLE_USE_SSL_FAILED:
//            return "Requested FTP SSL level failed";
//        case CURLE_SEND_FAIL_REWIND:
//            return "Sending the data requires a rewind that failed";
//        case CURLE_SSL_ENGINE_INITFAILED:
//            return "SSL crypto engine initialization failed";
//        case CURLE_LOGIN_DENIED:
//            return "User authentication failed";
//        case CURLE_TFTP_NOTFOUND:
//            return "TFTP server can't find requested file";
//        case CURLE_TFTP_PERM:
//            return "TFTP permission problem";
//        case CURLE_REMOTE_DISK_FULL:
//            return "Remote disk is full";
//        case CURLE_TFTP_ILLEGAL:
//            return "TFTP illegal operation";
//        case CURLE_TFTP_UNKNOWNID:
//            return "TFTP unknown transfer ID";
//        case CURLE_REMOTE_FILE_EXISTS:
//            return "File already exists";
//        case CURLE_TFTP_NOSUCHUSER:
//            return "TFTP no such user";
//        case CURLE_CONV_FAILED:
//            return "Conversion failed";
//        case CURLE_CONV_REQD:
//            return "Caller must register conversion callbacks using curl_easy_setopt options";
//        case CURLE_SSL_CACERT_BADFILE:
//            return "Could not load CACERT file, missing or wrong format";
//        case CURLE_REMOTE_FILE_NOT_FOUND:
//            return "Remote file not found";
//        case CURLE_SSH:
//            return "SSH error";
//        case CURLE_SSL_SHUTDOWN_FAILED:
//            return "SSL shutdown failed";
//        case CURLE_AGAIN:
//            return "Socket is not ready for send/recv, wait till it's ready and try again";
//        case CURLE_SSL_CRL_BADFILE:
//            return "Failed to load CRL file";
//        case CURLE_SSL_ISSUER_ERROR:
//            return "Issuer check failed";
//        case CURLE_FTP_PRET_FAILED:
//            return "PRET command failed";
//        case CURLE_RTSP_CSEQ_ERROR:
//            return "Mismatch of RTSP CSeq numbers";
//        case CURLE_RTSP_SESSION_ERROR:
//            return "Mismatch of RTSP Session Identifiers";
//        case CURLE_FTP_BAD_FILE_LIST:
//            return "Unable to parse FTP file list";
//        case CURLE_CHUNK_FAILED:
//            return "Chunk callback reported error";
//        case CURLE_NO_CONNECTION_AVAILABLE:
//            return "No connection available, the session will be queued";
//        case CURLE_SSL_PINNEDPUBKEYNOTMATCH:
//              return "SSL public key does not match pinned public key";
//        case CURLE_SSL_INVALIDCERTSTATUS:
//          return "SSL invalid certificate status";
//        case CURLE_HTTP2_STREAM:
//          return "Stream error in the HTTP/2 framing layer";
//        case CURLE_RECURSIVE_API_CALL:
//          return "An API function was called from inside a callback";
//        case CURLE_AUTH_ERROR:
//          return "Authentication error";
//        case CURLE_HTTP3:
//          return "HTTP/3 error";
//        case CURLE_QUIC_CONNECT_ERROR:
//          return "QUIC connection error";
//        default:
//          return "Unknown error";
//    }
}
