#include "qdropbox2.h"

QDropbox2::QDropbox2(QObject *parent)
    : QObject(parent),
      QNAM(this),
      lastErrorCode(QDropbox2::NoError),
      eventLoop(nullptr)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating dropbox api";
#endif

    init();
}

QDropbox2::QDropbox2(const QString& app_key, const QString& app_secret, QObject *parent, OAuthMethod method, QString url)
    : QObject(parent),
      QNAM(this),
      appKey(app_key),
      appSecret(app_secret),
      lastErrorCode(QDropbox2::NoError),
      eventLoop(nullptr)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating api with access token and method";
#endif

    init(url, method);
}

QDropbox2::QDropbox2(const QString& token, QObject *parent, OAuthMethod method, const QString& url)
    : QObject(parent),
      QNAM(this),
      accessToken_(token),
      lastErrorCode(QDropbox2::NoError),
      eventLoop(nullptr)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating api with access token and method";
#endif

    init(url, method);
}

void QDropbox2::init(const QString& api_url, QDropbox2::OAuthMethod oauth_method)
{
    setApiUrl(api_url);
    setAuthMethod(oauth_method);

    connect(&QNAM, &QNetworkAccessManager::finished, this, &QDropbox2::slot_networkRequestFinished);

    if(accessToken_.isEmpty() && !appKey.isEmpty() && !appSecret.isEmpty())
    {
        // we need to request a temporary access token
        tokenFromKeyAndSecret();  // this will set lastErrorCode on error
    }
}

QDropbox2::Error QDropbox2::error()
{
    return (QDropbox2::Error)lastErrorCode;
}

QString QDropbox2::errorString()
{
    return lastErrorMessage;
}

void QDropbox2::clearError()
{
    lastErrorCode = (int)QDropbox2::NoError;
    lastErrorMessage.clear();
}

void QDropbox2::setApiUrl(const QString& url)
{
    apiurl.setUrl(QString("/%1").arg(url));
    prepareApiUrl();
}

QString QDropbox2::apiUrl()
{
    return apiurl.toString();
}

void QDropbox2::setAuthMethod(OAuthMethod method)
{
    oauthMethod = method;
    prepareApiUrl();
    return;
}

QDropbox2::OAuthMethod QDropbox2::authMethod()
{
    return oauthMethod;
}

void QDropbox2::slot_networkRequestFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    QByteArray buff = reply->readAll();
    lastResponse = QString(buff);

#ifdef QTDROPBOX_DEBUG
    //qDebug() << "request " << nr << "finished.";
    qDebug() << "request was: " << reply->url().toString();
    qDebug() << "response: " << reply->bytesAvailable() << "bytes";
    qDebug() << "status code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    qDebug() << "== begin response ==" << endl << lastResponse << endl << "== end response ==";
#endif

    lastErrorCode = reply->error();
    if(replyMap.contains(reply))
    {
        CallbackPtr async_data(replyMap[reply]);
        if(async_data->callback)
            (this->*async_data->callback)(reply, async_data);
        replyMap.remove(reply);
    }
    else
    {
        if(lastErrorCode != QNetworkReply::NoError)
        {
#ifdef QTDROPBOX_DEBUG
            // debug information only - this should not happen, but if it does we
            // ignore replies when not waiting for anything
#endif
            lastErrorMessage = reply->errorString();

            if(lastErrorCode == QDROPBOX_V2_ERROR)
            {
                QJsonParseError jsonError;
                QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
                if(jsonError.error == QJsonParseError::NoError)
                {
                    cachedJson = json;
                    QJsonObject object = json.object();
                    if(object.contains("user_message"))
                        lastErrorMessage = object.value("user_message").toString();
                    else if(object.contains("error_summary"))
                        lastErrorMessage = object.value("error_summary").toString();
                }
            }

            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }

        stopEventLoop();
    }
}

void QDropbox2::prepareApiUrl()
{
    //if(oauthMethod == QDropbox::Plaintext)
    apiurl.setScheme("https");
    //else
    //  apiurl.setScheme("http");
}

bool QDropbox2::createAPIv2Request(QUrl request, QNetworkRequest& req, bool include_bearer)
{
    bool result = false;
    if(accessToken_.isEmpty() && include_bearer)
    {
        lastErrorCode = (int)QDropbox2::APIError;
        lastErrorMessage = "The authorizing access token has not been set.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error " << lastErrorCode << "(" << lastErrorMessage << ") in createAPIv2Reqeust()";
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        QString req_str = request.toString(QUrl::RemoveAuthority|QUrl::RemoveScheme);
        Q_ASSERT(req_str.startsWith("/"));

        req.setUrl(request);
        if(include_bearer)
        {
            QString bearer = QString("Bearer %1").arg(accessToken_);
            req.setRawHeader("Authorization", bearer.toUtf8());
        }

        result = true;
    }

    return result;
}

QNetworkReply* QDropbox2::sendPOST(QNetworkRequest& rq, QByteArray postdata)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "sendPOST() rq = " << rq.url().toString();
#endif

    return QNAM.post(rq, postdata);
}

QNetworkReply*  QDropbox2::sendGET(QNetworkRequest& rq)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "sendGET() rq = " << rq.url().toString();
#endif

    return QNAM.get(rq);
}

QString QDropbox2::signatureMethodString()
{
    QString sigmeth;
    switch(oauthMethod)
    {
        case QDropbox2::Plaintext:
            sigmeth = "PLAINTEXT";
            break;
        case QDropbox2::HMACSHA1:
            sigmeth = "HMAC-SHA1";
            break;
        default:
            lastErrorCode = (int)QDropbox2::UnknownAuthMethod;
            lastErrorMessage = QString("Authentication method %1 is unknown").arg(oauthMethod);
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
            break;
    }

    return sigmeth;
}

void QDropbox2::setAccessToken(const QString& token)
{
    accessToken_ = token;
}

QString QDropbox2::accessToken()
{
    return accessToken_;
}

QString QDropbox2::apiVersion()
{
    return "2.0";
}

QUrl QDropbox2::authorizeLink(const QString& appKey, const QString& redirect_uri, const QString& state)
{
    QUrl url;
    url.setScheme("https");
    url.setHost("www.dropbox.com");
    url.setPath("/oauth2/authorize");

    QUrlQuery query;
    query.addQueryItem("response_type", "code");
    query.addQueryItem("client_id", appKey);
    query.addQueryItem("redirect_uri", redirect_uri);

    if (!state.isEmpty())
        query.addQueryItem("state", state);

    url.setQuery(query);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "authorization URL = \"" << url.toEncoded() << "\"";
#endif

    return url;
}

bool QDropbox2::tokenFromKeyAndSecret()     // synchronous
{
    bool result = false;

    clearError();
    accessToken_.clear();

    QNetworkReply* reply;
    result = requestTokenViaOAuth1(reply);

    if(result)
    {
        startEventLoop();

        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
        result = (lastErrorCode == 0 && lastResponse.length());
        if(result)
        {
            if(jsonError.error == QJsonParseError::NoError)
            {
                cachedJson = json;
                QJsonObject object = json.object();
                if(object.contains("oauth2_token"))
                    accessToken_ = object.value("oauth2_token").toString();
            }
        }
        else if(lastErrorCode != 0)
        {
            lastErrorMessage.clear();
            lastErrorCode = (int)QDropbox2::UnknownAuthMethod;
            if(jsonError.error == QJsonParseError::NoError)
            {
                cachedJson = json;
                QJsonObject object = json.object();
                if(object.contains("error_summary"))
                    lastErrorMessage = object.value("error_summary").toString();
            }
            if(lastErrorMessage.isEmpty())
                lastErrorMessage = "An error occurred using the OAuth v1 interface.";
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
    }

    return !accessToken_.isEmpty();
}

bool QDropbox2::requestTokenViaOAuth1(QNetworkReply*& reply)
{
    clearError();

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL);
    url.setPath("/2/auth/token/from_oauth1");

    QNetworkRequest req;
    if(!createAPIv2Request(url, req, false))
        return false;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"oauth1_token\": \"%1\", \"oauth1_secret\": \"%2\"}")
                                .arg(appKey)
                                .arg(appSecret);

    reply = sendPOST(req);
    return reply != nullptr;
}

bool QDropbox2::revokeAccessToken()     // synchronous
{
    bool result = false;

    clearError();

    QNetworkReply* reply;
    result = requestTokenRevocation(reply);

    if(result)
    {
        startEventLoop();

        // "No return values" and "No errors" for this one.
        result = true;
    }
    else
    {
        lastErrorCode = (int)QDropbox2::APIError;
        lastErrorMessage = "An error occurred creating the network request.";
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }

    return result;
}

bool QDropbox2::requestTokenRevocation(QNetworkReply*& reply)
{
    clearError();

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL);
    url.setPath("/2/auth/token/revoke");

    QNetworkRequest req;
    if(!createAPIv2Request(url, req))
        return false;

    reply = sendPOST(req);
    return reply != nullptr;
}

bool QDropbox2::userInfo(QDropbox2User& info)   // synchronous
{
    bool result = false;

    clearError();

    QNetworkReply* reply;
    result = requestUserInfo(reply);

    if(result)
    {
        startEventLoop();

        result = (lastErrorCode == 0 && lastResponse.length());
        if(result)
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
            if(jsonError.error == QJsonParseError::NoError)
            {
                cachedJson = json;
                QJsonObject object = json.object();
                QDropbox2User a(object, this);
                info = a;
            }
            else
            {
                lastErrorCode = (int)QDropbox2::APIError;
                lastErrorMessage  = "Dropbox API did not send correct answer for account information.";
                emit signal_errorOccurred(lastErrorCode, lastErrorMessage);

                result = false;
            }
        }
    }

    return result;
}

bool QDropbox2::userInfo()      // asynchronous
{
    bool result = false;

    clearError();

    QNetworkReply* reply;
    result = requestUserInfo(reply);

    if(result)
    {
        CallbackPtr reply_data(new CallbackData());
        reply_data->callback = &QDropbox2::userInfoCallback;
        replyMap[reply] = reply_data;
    }

    return result;
}

bool QDropbox2::requestUserInfo(QNetworkReply*& reply)
{
    clearError();

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL);
    url.setPath("/2/users/get_current_account");

    QNetworkRequest req;
    if(!createAPIv2Request(url, req))
        return false;

    reply = sendPOST(req);
    return reply != nullptr;
}

void QDropbox2::userInfoCallback(QNetworkReply* /*reply*/, CallbackPtr /*data*/)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "== user info ==" << lastResponse << "== user info end ==";
#endif

    QJsonParseError jsonError;
    QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
    if(jsonError.error != QJsonParseError::NoError)
    {
        lastErrorCode = (int)QDropbox2::APIError;
        lastErrorMessage  = "Dropbox API did not send correct answer for account information.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        cachedJson = json;
        QDropbox2User info(json.object(), this);
        emit signal_userInfoReceived(info);
    }
}

bool QDropbox2::usageInfo(QDropbox2Usage& info)     // synchronous
{
    bool result = false;

    clearError();

    QNetworkReply* reply;
    result = requestUsageInfo(reply);

    if(result)
    {
        startEventLoop();

        result = (lastErrorCode == 0 && lastResponse.length());
        if(result)
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
            if(jsonError.error == QJsonParseError::NoError)
            {
                cachedJson = json;
                QJsonObject object = json.object();
                QDropbox2Usage a(object, this);
                info = a;
            }
            else
            {
                lastErrorCode = (int)QDropbox2::APIError;
                lastErrorMessage  = "Dropbox API did not send correct answer for account information.";
                emit signal_errorOccurred(lastErrorCode, lastErrorMessage);

                result = false;
            }
        }
    }

    return result;
}

bool QDropbox2::usageInfo()     // asynchronous
{
    bool result = false;

    clearError();

    QNetworkReply* reply;
    result = requestUsageInfo(reply);

    if(result)
    {
        CallbackPtr reply_data(new CallbackData());
        reply_data->callback = &QDropbox2::usageInfoCallback;
        replyMap[reply] = reply_data;
    }

    return result;
}

bool QDropbox2::requestUsageInfo(QNetworkReply*& reply)
{
    clearError();

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL);
    url.setPath("/2/users/get_space_usage");

    QNetworkRequest req;
    if(!createAPIv2Request(url, req))
        return false;

    reply = sendPOST(req);
    return reply != nullptr;
}

void QDropbox2::usageInfoCallback(QNetworkReply* /*reply*/, CallbackPtr /*data*/)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "== usage info ==" << lastResponse << "== usage info end ==";
#endif

    QJsonParseError jsonError;
    QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
    if(jsonError.error != QJsonParseError::NoError)
    {
        lastErrorCode = (int)QDropbox2::APIError;
        lastErrorMessage  = "Dropbox API did not send correct answer for account information.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            cachedJson = json;
            QJsonObject object = json.object();
            QDropbox2Usage usage(object, this);
            emit signal_usageInfoReceived(usage);
        }
        else
        {
            lastErrorCode = (int)QDropbox2::APIError;
            lastErrorMessage  = "Dropbox API did not send correct answer for account information.";
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
    }
}

void QDropbox2::startEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox::startEventLoop()";
#endif
    if(eventLoop == nullptr)
        eventLoop = new QEventLoop(this);
    eventLoop->exec();
}

void QDropbox2::stopEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox::stopEventLoop()";
#endif
    if(eventLoop == nullptr)
        return;
#ifdef QTDROPBOX_DEBUG
    qDebug() << "loop ended";
#endif
    eventLoop->exit();
}
