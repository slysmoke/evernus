#ifndef QTDROPBOX_GLOBAL_H
#define QTDROPBOX_GLOBAL_H

#include <QtCore/qglobal.h>

#define QTDROPBOXSHARED_EXPORT

#ifndef QDROPBOX_HTTP_ERROR_CODES
#define QDROPBOX_HTTP_ERROR_CODES
const qint32 QDROPBOX_ERROR_BAD_INPUT           = 400;
const qint32 QDROPBOX_ERROR_EXPIRED_TOKEN       = 401;
const qint32 QDROPBOX_ERROR_BAD_OAUTH_REQUEST   = 403;
const qint32 QDROPBOX_ERROR_FILE_NOT_FOUND      = 404;
const qint32 QDROPBOX_ERROR_WRONG_METHOD        = 405;
const qint32 QDROPBOX_ERROR_REQUEST_CAP         = 503;
const qint32 QDROPBOX_ERROR_USER_OVER_QUOTA     = 507;
#endif

#endif // QTDROPBOX_GLOBAL_H