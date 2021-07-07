#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


#include <QString>
#include "downloadmanager.h"


void download_file(QString url, QString &save_file_name, download_progress &info);

QString get_file_name_from_url(QString url);

void send_post_request(QString url, wafah_data *data);



#endif // HTTPREQUEST_H
