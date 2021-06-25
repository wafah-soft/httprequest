#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


#include <QString>

typedef void(__stdcall *download_info)(int percent, QString info, qint64 total_byte);

void download_file(QString url, QString &save_file_name, download_info &info);

#endif // HTTPREQUEST_H
