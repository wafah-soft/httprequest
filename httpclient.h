#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <iostream>
#include "httpclientdef.h"


int http_post(wafah_handler *handler, std::string url, wafah_data *data);

#endif // HTTPCLIENT_H
