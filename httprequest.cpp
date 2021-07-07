#include "httprequest.h"
#include <QUrl>

DownloadManager mgr;


void download_file(QString url, QString &save_file_name, download_progress &info)
{
    mgr.setOutput_filename(save_file_name);
    mgr.set_download_callback(info);
    mgr.do_download(QUrl(url));
}

QString get_file_name_from_url(QString url)
{
    auto path = QUrl(url).path();
    auto file_name = QFileInfo(path).fileName();
    return file_name;
}

void send_post_request(QString url, wafah_data *data)
{
    mgr.send_post(url, data);
}
