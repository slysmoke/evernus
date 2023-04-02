#include <QFileInfo>
#include <QLocale>

#include "qdropbox2entityinfo.h"

QDropbox2EntityInfo::QDropbox2EntityInfo(QObject *parent)
    : QObject(parent)
{
  init();
}

QDropbox2EntityInfo::QDropbox2EntityInfo(const QJsonObject& jsonData, QObject *parent)
    : QObject(parent)
{
    init(jsonData);
}

QDropbox2EntityInfo::QDropbox2EntityInfo(const QDropbox2EntityInfo &other)
    : QObject(0)
{
    init();
    copyFrom(other);
}

QDropbox2EntityInfo::~QDropbox2EntityInfo()
{
}

void QDropbox2EntityInfo::copyFrom(const QDropbox2EntityInfo &other)
{
    setParent(other.parent());

    _id             = other._id;
    _size           = other._size;
    _bytes          = other._bytes;
    _serverModified = other._serverModified;
    _clientModified = other._clientModified;
    _path           = other._path;
    _filename       = other._filename;
    _revisionHash   = other._revisionHash;
    _isDir          = other._isDir;
    _isShared       = other._isShared;
    _isDeleted      = other._isDeleted;
}

QDropbox2EntityInfo &QDropbox2EntityInfo::operator=(const QDropbox2EntityInfo &other)
{
    copyFrom(other);
    return *this;
}

void QDropbox2EntityInfo::init(const QJsonObject& jsonData)
{
    if(!jsonData.isEmpty())
    {
        _id             = jsonData.value("id").toString();
        _clientModified = getTimestamp("client_modified");
        _serverModified = getTimestamp("server_modified");
        _revisionHash   = jsonData.value("rev").toString();
        _bytes          = jsonData.value("size").toInt();
        _size           = QString::number(_bytes);
        _path           = jsonData.value("path_display").toString();
        _isShared       = jsonData.contains("sharing_info");
        _isDir          = jsonData.value(".tag").toString().compare("folder") == 0;
        _isDeleted      = jsonData.value(".tag").toString().compare("deleted") == 0;

        QFileInfo info(_path);
        _filename = info.fileName();
    }
    else
    {
        _id             = "";
        _size           = "";
        _bytes          = 0;
        _serverModified = QDateTime::currentDateTime();
        _clientModified = QDateTime::currentDateTime();
        _path           = "";
        _filename       = "";
        _revisionHash   = "";
        _isDir          = false;
        _isShared       = false;
        _isDeleted      = false;
    }
}

QDateTime QDropbox2EntityInfo::getTimestamp(QString value)
{
    // APIv2: 2015-05-12T15:50:38Z
    const QString dtFormat = "\"yyyy-MM-ddTHH:mm:ssZ\"";

    QDateTime res = QLocale(QLocale::English).toDateTime(value, dtFormat);
    res.setTimeSpec(Qt::UTC);

    return res;
}

QString QDropbox2EntityInfo::size() const
{
    QLocale local;
    return local.toString(_bytes);
}
