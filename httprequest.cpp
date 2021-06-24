#include "httprequest.h"
#include "downloadmanager.h"
#include <QUrl>

void download_file(QString url, QString &save_file_name, download_info *info)
{
    DownloadManager mgr;
    mgr.do_download(QUrl(url));
    mgr.setOutput_filename(save_file_name);
    mgr.set_download_callback(*info);
}
