#include "qdropbox2file.h"

QDropbox2File::QDropbox2File(QObject *parent)
    : QIODevice(parent),
      IQDropbox2Entity(),
      QNAM(this)
{
    init(nullptr, "");
}

QDropbox2File::QDropbox2File(QDropbox2 *api, QObject *parent)
    : QIODevice(parent),
      IQDropbox2Entity(),
      QNAM(this)
{
    init(api, "");
}

QDropbox2File::QDropbox2File(const QString& filename, QDropbox2 *api, QObject *parent)
    : QIODevice(parent),
      IQDropbox2Entity(),
      QNAM(this)
{
    init(api, filename);
}

QDropbox2File::QDropbox2File(const QDropbox2File& source)
    : QIODevice(source.parent()),
      IQDropbox2Entity(source)
{
    init(source._api, source._filename);
}

QDropbox2File::~QDropbox2File()
{
    if(_buffer)
        delete _buffer;
    if(eventLoop)
        delete eventLoop;
}

void QDropbox2File::init(QDropbox2 *api, const QString& filename, qint64 threshold)
{
    if(filename.compare("/") == 0 || filename.isEmpty())
    {
        lastErrorCode = QDropbox2::APIError;
        lastErrorMessage = "Filename cannot be root ('/')";
    }
    else
    {
        _api              = api;
        _buffer           = nullptr;
        _filename         = filename;
        eventLoop         = nullptr;
        bufferThreshold   = threshold;
        overwrite_        = true;
        rename            = false;
        _metadata         = nullptr;
        lastErrorCode     = 0;
        lastErrorMessage  = "";
        position          = 0;
        currentThreshold  = 0;
        fileExists        = false;

        if(api)
            accessToken = api->accessToken();

        connect(&QNAM, &QNetworkAccessManager::finished, this, &QDropbox2File::slot_networkRequestFinished);
    }
}

int QDropbox2File::error()
{
    return lastErrorCode;
}

QString QDropbox2File::errorString()
{
    return lastErrorMessage;
}

bool QDropbox2File::isSequential() const
{
    return true;
}

bool QDropbox2File::open(QIODevice::OpenMode mode)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::open(...)";
#endif
    if(!QIODevice::open(mode))
        return result;

  /*  if(isMode(QIODevice::NotOpen))
        return true; */

    if(!_buffer)
        _buffer = new QByteArray();

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File: opening file";
#endif

    // clear buffer and reset position if this file was opened in write mode
    // with truncate - or if append was not set
    if(isMode(QIODevice::WriteOnly) &&
       (isMode(QIODevice::Truncate) || !isMode(QIODevice::Append))
      )
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File: _buffer cleared.";
#endif
        _buffer->clear();
        position = 0;
        result = true;
    }
    else
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File: reading file content";
#endif
        if(getFile(_filename))  // this will return true if the file doesn't already exist
        {
            if(isMode(QIODevice::WriteOnly)) // write mode here means append
                position = _buffer->size();
            else if(isMode(QIODevice::ReadOnly)) // read mode here means start at the beginning
                position = 0;

            result = (lastErrorCode == 0 || lastErrorCode == 200 || fileExists);
            if(fileExists)
                obtainMetadata();
        }
    }

    return result;
}

void QDropbox2File::close()
{
    if(isMode(QIODevice::WriteOnly) && _buffer->length())
        flush();
    QIODevice::close();
}

void QDropbox2File::setApi(QDropbox2 *dropbox)
{
    _api = dropbox;
}

void QDropbox2File::setFilename(const QString& filename)
{
    _filename = filename;
}

bool QDropbox2File::flush()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::flush()";
#endif

    return putFile();
}

bool QDropbox2File::event(QEvent *event)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "processing event: " << event->type();
#endif
    return QIODevice::event(event);
}

//void QDropbox2File::setFlushThreshold(qint64 num)
//{
//    if(num < 0)
//        num = MaxSingleUpload;
//    bufferThreshold = num;
//}

void QDropbox2File::setOverwrite(bool overwrite)
{
    overwrite_ = overwrite;
}

void QDropbox2File::setRenaming(bool rename)
{
    this->rename = rename;
}

qint64 QDropbox2File::readData(char *data, qint64 maxlen)
{
    if(!maxlen)
        return maxlen;      // we do no "post-reading operations"

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::readData(...), maxlen = " << maxlen;
    QString buff_str = QString(*_buffer);
    //qDebug() << "old bytes = " << _buffer->toHex() << ", str: " << buff_str <<  endl;
    qDebug() << "old size = " << _buffer->size();
#endif

    if(_buffer->size() == 0 || position >= _buffer->size())
        return 0;

    if(_buffer->size() < maxlen)
        maxlen = _buffer->size();

    QByteArray tmp = _buffer->mid(position, maxlen);
    const qint64 read = tmp.size();
    memcpy(data, tmp.data(), read);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "new size = " << _buffer->size();
    //qDebug() << "new bytes = " << _buffer->toHex();
#endif

    position += read;

    return read;
}

qint64 QDropbox2File::writeData(const char *data, qint64 len)
{
    int written_bytes = 0;

    qint64 oldlen = _buffer->size();
    _buffer->insert(position, data, len);

    //// flush if the threshold is reached
    //if(currentThreshold > bufferThreshold)
    //    flush();

    currentThreshold += len;
    written_bytes = len;

    if(_buffer->size() != oldlen+len)
        written_bytes = (oldlen-_buffer->size());

    position += written_bytes;

    return written_bytes;
}

void QDropbox2File::slot_networkRequestFinished(QNetworkReply *reply)
{
    reply->deleteLater();

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
        if(lastErrorCode == QNetworkReply::NoError)
        {
            QByteArray buff = reply->readAll();
            lastResponse = QString(buff);
    #ifdef QTDROPBOX_DEBUG
            qDebug() << "QDropbox2Folder::slot_networkRequestFinished(...)";
            qDebug() << "request was: " << reply->url().toString();
            qDebug() << "response: " << reply->bytesAvailable() << "bytes";
            qDebug() << "status code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
            qDebug() << "== begin response ==" << endl << lastResponse << endl << "== end response ==";
    #endif
        }
        else
            lastErrorMessage = reply->errorString();

        stopEventLoop();
    }
}

bool QDropbox2File::isMode(QIODevice::OpenMode mode)
{
    return ((openMode() & mode) == mode);
}

QNetworkReply* QDropbox2File::sendPOST(QNetworkRequest& rq, QByteArray& postdata)
{
    QNetworkReply *reply = QNAM.post(rq, postdata);
    connect(this, &QDropbox2File::signal_operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::uploadProgress, this, &QDropbox2File::slot_uploadProgress);
    return reply;
}

QNetworkReply* QDropbox2File::sendGET(QNetworkRequest& rq)
{
    QNetworkReply *reply = QNAM.get(rq);
    connect(this, &QDropbox2File::signal_operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::downloadProgress, this, &QDropbox2File::signal_downloadProgress);
    return reply;
}

bool QDropbox2File::getFile(const QString& filename)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::getFileContent(...)";
#endif
    QUrl url;
    url.setUrl(QDROPBOX2_CONTENT_URL, QUrl::StrictMode);
    url.setPath("/2/files/download");

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setRawHeader("Dropbox-API-arg", QString("{ \"path\": \"%1\" }")
                                                .arg((filename.compare("/") == 0) ? "" : filename).toUtf8());

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::getFileContent " << url.toString();
#endif

    QNetworkReply* reply = sendGET(req);

    CallbackPtr reply_data(new CallbackData);
    reply_data->callback = &QDropbox2File::resultGetFile;
    replyMap[reply] = reply_data;

    startEventLoop();

    fileExists = !lastErrorMessage.contains("path/not_found");
    result = (lastErrorCode == 0 || lastErrorCode == 200 || lastErrorCode == 206 || !fileExists);
    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::getFileContent ReadError: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }

    return result;
}

void QDropbox2File::resultGetFile(QNetworkReply *reply, CallbackPtr /*reply_data*/)
{
    lastErrorCode = 0;

    QByteArray response = reply->readAll();
    QString resp_str;

//#ifdef QTDROPBOX_DEBUG
//    resp_str = QString(response.toHex());
//    qDebug() << "QDropbox2File::replyFileContent response = " << resp_str;
//
//#endif

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == QDROPBOX_V2_ERROR)
    {
        resp_str = QString(response);
        lastErrorMessage = "";

        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(resp_str.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject object = json.object();
            if(object.contains("user_message"))
                lastErrorMessage = object.value("user_message").toString();
            else if(object.contains("error_summary"))
                lastErrorMessage = object.value("error_summary").toString();
        }

        lastErrorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::replyFileContent jason.null = " << json.isNull();
#endif

        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        _buffer->clear();
        _buffer->append(response);
        emit readyRead();
    }

    stopEventLoop();
}

bool QDropbox2File::putFile()
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::putFile()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_CONTENT_URL, QUrl::StrictMode);
    if(_buffer->length() <= MaxSingleUpload)
        url.setPath("/2/files/upload");
    else
        url.setPath("/2/files/upload_session/start");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    QString json = QString("{ \"path\": \"%1\", \"mode\": \"%2\", \"autorename\": %3, \"mute\": true }")
                                        .arg(_filename)
                                        .arg(overwrite_ ? "overwrite" : "add")
                                        .arg(rename ? "true" : "false");
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::Dropbox-API-arg " << json;
    qDebug() << "QDropbox2File::putFile " << url.toString();
#endif
    QNetworkReply* reply = nullptr;

    if(_buffer->length() <= MaxSingleUpload)
    {
        req.setRawHeader("Dropbox-API-arg", json.toUtf8());
        reply = sendPOST(req, *_buffer);
    }
    else
    {
        QString session_json = QString("{ \"close\": false }");
        req.setRawHeader("Dropbox-API-arg", session_json.toUtf8());

        QByteArray dummy_data;
        reply = sendPOST(req, dummy_data);

        session_starts[reply] = json;
    }

    // "{ \"path\": \"%1\", \"mode\": \"overwrite\", \"autorename\": %2, \"mute\": true }"
    // "{ \"path\": \"%1\", \"mode\": \"update\", \"autorename\": %2, \"mute\": true }"

    CallbackPtr reply_data(new CallbackData);
    reply_data->callback = &QDropbox2File::resultPutFile;
    replyMap[reply] = reply_data;

    startEventLoop();

    result = (lastErrorCode == 0);
    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::putFile WriteError: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        // we wrote the whole file, so reset
        _buffer->clear();
        position = 0;
        currentThreshold = 0;
    }

    return result;
}

void QDropbox2File::resultPutFile(QNetworkReply *reply, CallbackPtr /*reply_data*/)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::replyFileWrite(...)";
#endif

    lastErrorCode = 0;

    QByteArray response = reply->readAll();
    QString resp_str;

#ifdef QTDROPBOX_DEBUG
    resp_str = response;
    qDebug() << "QDropbox2File::resultPutFile response = " << resp_str;
#endif

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == QDROPBOX_V2_ERROR)
    {
        resp_str = QString(response);
        lastErrorMessage = "";

        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(resp_str.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject object = json.object();
            if(object.contains("user_message"))
                lastErrorMessage = object.value("user_message").toString();
            else if(object.contains("error_summary"))
                lastErrorMessage = object.value("error_summary").toString();
        }

        lastErrorCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::resultPutFile jason.null = " << json.isNull();
#endif

        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        if(_metadata)
            _metadata->deleteLater();
        _metadata = nullptr;

        QJsonObject object;
        if(!response.isNull() && !response.isEmpty()) // might be the case for an upload session
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(response, &jsonError);
            if(jsonError.error == QJsonParseError::NoError && !json.isEmpty())
                object = json.object();
        }

        SessionPtr sd;
        if(session_starts.contains(reply))
        {
            // we've initiated a new upload session

            sd = SessionPtr(new SessionData());

            // this will be an upload_session id on the first response
            Q_ASSERT(object.contains("session_id"));
            sd->session_id = object["session_id"].toString();
            sd->session_parameters = session_starts[reply];
            sd->session_offset = 0;

            session_starts.remove(reply);
        }
        else if(upload_sessions.contains(reply))
        {
            // continue an active session
            sd = upload_sessions[reply];
            // set this here so upload progress uses correct values
            sd->session_offset += sd->session_payload;

            upload_sessions.remove(reply);
        }

        // do we have an active upload session?
        if(!sd.isNull())
        {
            int remaining = _buffer->length() - sd->session_offset;

            if(remaining)
            {
                QUrl url;
                url.setUrl(QDROPBOX2_CONTENT_URL, QUrl::StrictMode);

                // have we reached the end of the buffer?
                if(remaining <= MaxSingleUpload)
                    url.setPath("/2/files/upload_session/finish");      // upload the final chunk
                else
                    url.setPath("/2/files/upload_session/append_v2");   // upload the next chunk

                Q_ASSERT(url.isValid());

                QNetworkRequest req;
                if(!_api->createAPIv2Request(url, req))
                    return;

                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
                QString json = QString("{ \"cursor\": { \"session_id\": \"%1\", \"offset\": %2 }, ")
                                        .arg(sd->session_id)
                                        .arg(sd->session_offset);

                if(remaining <= MaxSingleUpload)
                    json += QString("\"commit\": %3 }").arg(sd->session_parameters);
                else
                    json += QStringLiteral("\"close\": false }");

                req.setRawHeader("Dropbox-API-arg", json.toUtf8());

                sd->session_payload = (remaining < MaxSingleUpload) ? remaining : MaxSingleUpload;

                QByteArray session_data = _buffer->mid(sd->session_offset, sd->session_payload);
                QNetworkReply* new_reply = sendPOST(req, session_data);

                upload_sessions[new_reply] = sd;

                CallbackPtr reply_data(new CallbackData);
                reply_data->callback = &QDropbox2File::resultPutFile;
                replyMap[new_reply] = reply_data;

                // not very structured, but we need to keep the event loop
                // going, so we bail here...

                return;
            }
            else    // we're done.
                // 'object' should be metadata from an "upload_session/finish"
                _metadata = new QDropbox2EntityInfo(object, this);
        }
        else    // regular upload
            _metadata = new QDropbox2EntityInfo(object, this);

        if(_metadata)
            emit bytesWritten(_buffer->size());
    }

    stopEventLoop();
}

void QDropbox2File::startEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::startEventLoop()";
#endif
    if(!eventLoop)
        eventLoop = new QEventLoop(this);
    eventLoop->exec();
}

void QDropbox2File::stopEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::stopEventLoop()";
#endif
    if(!eventLoop)
        return;
    eventLoop->exit();
}

QDropbox2EntityInfo QDropbox2File::metadata()
{
    obtainMetadata();

    if(_metadata)
        lastHash = _metadata->revisionHash();

    return _metadata ? *_metadata : QDropbox2EntityInfo();
}

void QDropbox2File::obtainMetadata()
{
    if(_metadata)
        _metadata->deleteLater();
    _metadata = nullptr;

    // APIv2 Note: Metadata for the root folder is unsupported.
    if(!_filename.compare("/") || _filename.isEmpty())
    {
        lastErrorCode = QDropbox2::APIError;
        lastErrorMessage = "Metadata for the root folder is unsupported.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        QUrl url;
        url.setUrl(QDROPBOX2_API_URL);
        url.setPath(QString("/2/files/get_metadata"));

#ifdef QTDROPBOX_DEBUG
        qDebug() << "metadata = \"" << url.toEncoded() << "\"";
#endif

        QNetworkRequest req;
        if(!_api->createAPIv2Request(url, req))
            return;
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QString json = QString("{\"path\": \"%1\", \"include_media_info\": true, \"include_deleted\": true, \"include_has_explicit_shared_members\": true}")
                                    .arg(_filename);
#ifdef QTDROPBOX_DEBUG
        qDebug() << "postdata = \"" << json << "\"";
#endif
        QByteArray postdata = json.toUtf8();
        (void)sendPOST(req, postdata);

        startEventLoop();

        if(lastErrorCode != 0)
        {
#ifdef QTDROPBOX_DEBUG
            qDebug() << "QDropbox2File::requestRemoval error: " << lastErrorCode << lastErrorMessage;
#endif
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
        else
        {
            QJsonParseError jsonError;
            QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
            if(jsonError.error == QJsonParseError::NoError)
            {
                QJsonObject object = json.object();
                _metadata = new QDropbox2EntityInfo(object);
            }
            else
            {
                lastErrorCode = QDropbox2::APIError;
                lastErrorMessage = "Dropbox API did not send correct answer for file/directory metadata.";
#ifdef QTDROPBOX_DEBUG
                qDebug() << "error: " << lastErrorMessage;
#endif
                emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
            }
        }
    }
}

bool QDropbox2File::hasChanged()
{
    if(lastHash.isEmpty())
    {
        obtainMetadata();
        if(_metadata)
            lastHash = _metadata->revisionHash();
        return false;
    }

    bool result = false;

    // get updated information
    obtainMetadata();

    if(_metadata)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::hasChanged() local  revision hash = " << lastHash;
        qDebug() << "QDropbox2File::hasChanged() remote revision hash = " << _metadata->revisionHash();
#endif
        result = lastHash.compare(_metadata->revisionHash()) != 0;
        lastHash = _metadata->revisionHash();
    }

    return result;
}

//void QDropbox2File::obtainMetadata()
//{
//    // get metadata of this file
//    if(_metadata)
//        _metadata->deleteLater();
//    _metadata = new QDropbox2EntityInfo(_api->requestMetadataAndWait(_filename).strContent(), this);
//    if(!_metadata->isValid())
//        _metadata->clear();
//}

bool QDropbox2File::revisions(QDropbox2File::RevisionsList& revisions, quint64 max_results)
{
    bool result = false;
    revisions.clear();

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::revisions()";
#endif

    QNetworkReply* reply;
    result = getRevisions(reply, max_results);

    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2Folder::contents error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        result = true;

        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject object = json.object();
            if(object.contains("entries"))
            {
                QJsonArray data = object.value("entries").toArray();
                if(data.count())
                {
                    foreach(const QJsonValue& entry, data)
                    {
                        QJsonObject obj = entry.toObject();
                        revisions.append(QDropbox2EntityInfo(entry.toObject()));
                    }
                }
            }
        }
        else
        {
            result = false;

            lastErrorCode = QDropbox2::APIError;
            lastErrorMessage = "Dropbox API did not send correct answer for revision data.";
#ifdef QTDROPBOX_DEBUG
            qDebug() << "error: " << lastErrorMessage;
#endif
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
    }

    return result;
}

bool QDropbox2File::revisions(quint64 max_results)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::revisions()";
#endif

    QNetworkReply* reply;
    bool result = getRevisions(reply, max_results);

    CallbackPtr reply_data(new CallbackData);
    reply_data->callback = &QDropbox2File::revisionsCallback;
    replyMap[reply] = reply_data;

    return result;
}

void QDropbox2File::revisionsCallback(QNetworkReply* /*reply*/, CallbackPtr /*reply_data*/)
{
    if(lastErrorCode)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2Folder::revisionsCallback error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        RevisionsList revisions_results;

        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject object = json.object();
            if(object.contains("entries"))
            {
                QJsonArray data = object.value("entries").toArray();
                if(data.count())
                {
                    foreach(const QJsonValue& entry, data)
                        revisions_results.append(QDropbox2EntityInfo(entry.toObject()));
                }
            }

            emit signal_revisionsResult(revisions_results);
        }
        else
        {
            lastErrorCode = QDropbox2::APIError;
            lastErrorMessage = "Dropbox API did not send correct answer for revision data.";
#ifdef QTDROPBOX_DEBUG
            qDebug() << "error: " << lastErrorMessage;
#endif
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
    }
}

bool QDropbox2File::getRevisions(QNetworkReply*& reply, quint64 max_results, bool async)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::getRevisions()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL, QUrl::StrictMode);
    url.setPath("/2/files/list_revisions");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"path\": \"%1\", \"limit\": %2}")
                            .arg(_filename)
                            .arg(max_results);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "postdata = \"" << json << "\"";
#endif
    QByteArray postdata = json.toUtf8();
    reply = sendPOST(req, postdata);

    if(async)
        return true;

    startEventLoop();

    result = (lastErrorCode == 0);
    return result;
}

bool QDropbox2File::seek(qint64 pos)
{
    if(pos > _buffer->size())
        return false;

    QIODevice::seek(pos);
    position = pos;
    return true;
}

bool QDropbox2File::reset()
{
    QIODevice::reset();
    position = 0;
    return true;
}

void QDropbox2File::slot_abort()
{
    emit signal_operationAborted();
}

void QDropbox2File::slot_uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(!session_starts.contains(reply) && !upload_sessions.contains(reply))
        emit signal_uploadProgress(bytesSent, bytesTotal);
    else if(bytesTotal && !session_starts.contains(reply))    // no progress with a start
    {
        // we have to adjust the values based on the progress
        // of the upload session.  bytesSent/bytesTotal are only
        // for the current chunk.

        SessionPtr sd = upload_sessions[reply];

        // 'sd->session_payload' should be equal to 'bytesTotal'

        emit signal_uploadProgress(sd->session_offset + bytesSent, _buffer->length());
    }
}

qint64 QDropbox2File::bytesAvailable() const
{
    return _buffer->size();
}

bool QDropbox2File::remove(bool permanently)
{
    return requestRemoval(permanently);
}

bool QDropbox2File::requestRemoval(bool permanently)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::requestRemoval()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL, QUrl::StrictMode);
    if(permanently)
        url.setPath("/2/files/permanently_delete");
    else
        url.setPath("/2/files/delete");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"path\": \"%1\"}")
                                .arg((_filename.compare("/") == 0) ? "" : _filename);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "postdata = \"" << json << "\"";
#endif
    QByteArray postdata = json.toUtf8();
    (void)sendPOST(req, postdata);

    startEventLoop();

    result = (lastErrorCode == 0);
    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::requestRemoval error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }

    return result;
}

bool QDropbox2File::move(const QString& to_path)
{
    return requestMove(to_path);
}

bool QDropbox2File::requestMove(const QString& to_path)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::requestMove()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL, QUrl::StrictMode);
    url.setPath("/2/files/move");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"from_path\": \"%1\", \"to_path\": \"%2\", \"autorename\": %3, \"allow_shared_folder\": false}")
                                    .arg(_filename)
                                    .arg(to_path)
                                    .arg(rename ? "true" : "false");

#ifdef QTDROPBOX_DEBUG
    qDebug() << "postdata = \"" << json << "\"";
#endif
    QByteArray postdata = json.toUtf8();
    (void)sendPOST(req, postdata);

    startEventLoop();

    result = (lastErrorCode == 0);
    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::requestMove error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }

    return result;
}

// TODO: Collapse copy into move
bool QDropbox2File::copy(const QString& to_path)
{
    return requestCopy(to_path);
}

bool QDropbox2File::requestCopy(const QString& to_path)
{
    bool result = false;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::requestCopy()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL, QUrl::StrictMode);
    url.setPath("/2/files/copy");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"from_path\": \"%1\", \"to_path\": \"%2\", \"autorename\": %3, \"allow_shared_folder\": false}")
                                    .arg(_filename)
                                    .arg(to_path)
                                    .arg(rename ? "true" : "false");

#ifdef QTDROPBOX_DEBUG
    qDebug() << "postdata = \"" << json << "\"";
#endif
    QByteArray postdata = json.toUtf8();
    (void)sendPOST(req, postdata);

    startEventLoop();

    result = (lastErrorCode == 0);
    if(!result)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::requestCopy error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }

    return result;
}

QUrl QDropbox2File::temporaryLink()
{
    return requestStreamingLink();
}

QUrl QDropbox2File::requestStreamingLink()
{
    QUrl result;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox2File::requestStreamingLink()";
#endif

    QUrl url;
    url.setUrl(QDROPBOX2_API_URL, QUrl::StrictMode);
    url.setPath("/2/files/get_temporary_link");

    Q_ASSERT(url.isValid());

    QNetworkRequest req;
    if(!_api->createAPIv2Request(url, req))
        return result;

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QString json = QString("{\"path\": \"%1\"}").arg(_filename);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "postdata = \"" << json << "\"";
#endif
    QByteArray postdata = json.toUtf8();
    (void)sendPOST(req, postdata);

    startEventLoop();

    if(lastErrorCode != 0)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropbox2File::requestStreamingLink error: " << lastErrorCode << lastErrorMessage;
#endif
        emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
    }
    else
    {
        QJsonParseError jsonError;
        QJsonDocument json = QJsonDocument::fromJson(lastResponse.toUtf8(), &jsonError);
        if(jsonError.error == QJsonParseError::NoError)
        {
            QJsonObject object = json.object();
            if(object.contains("link"))
                result.setUrl(object.value("link").toString());
        }
        else
        {
            lastErrorCode = QDropbox2::APIError;
            lastErrorMessage = "Dropbox API did not send correct answer for temporary link data.";
#ifdef QTDROPBOX_DEBUG
            qDebug() << "error: " << lastErrorMessage;
#endif
            emit signal_errorOccurred(lastErrorCode, lastErrorMessage);
        }
    }

    return result;
}
