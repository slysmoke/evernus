#pragma once

#include <QtCore/QString>
#include <QtCore/qglobal.h>

const int MaxSingleUpload = (150*1024*1024);

#ifndef QDROPBOX_V2_HTTP_ERROR_CODES
#define QDROPBOX_V2_HTTP_ERROR_CODES

// Dropbox APIv2 error handling
// https://blogs.dropbox.com/developers/2015/04/a-preview-of-the-new-dropbox-api-v2/
//
// "API v2 will always return a 409 status code with a stable and documented error
// identifier in the body. We chose 409 because, unlike many other error codes, it
// doesn't have any specific meaning in the HTTP spec. This ensures that HTTP
// intermediaries, such as proxies or client libraries, will relay it along untouched."

const qint32 QDROPBOX_V2_ERROR                    = 409;
#endif

#ifndef QDROPBOX_V2_URLS
#define QDROPBOX_V2_URLS
const QString QDROPBOX2_API_URL     = "https://api.dropboxapi.com";
const QString QDROPBOX2_CONTENT_URL = "https://content.dropboxapi.com";
const QString QDROPBOX2_NOTIFY_URL  = "https://notify.dropboxapi.com";
#endif
