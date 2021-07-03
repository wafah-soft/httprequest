#include "httprequest.h"
#include "downloadmanager.h"
#include <QUrl>

DownloadManager mgr;


void download_file(QString url, QString &save_file_name, download_info &info)
{
    mgr.setOutput_filename(save_file_name);
    mgr.set_download_callback(info);
    mgr.do_download(QUrl(url));
}

std::string get_file_name(std::string url)
{
    auto path = QUrl(url.c_str()).path();
    auto file_name = QFileInfo(path).fileName();
    return file_name.toStdString();
}
