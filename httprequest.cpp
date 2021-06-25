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
