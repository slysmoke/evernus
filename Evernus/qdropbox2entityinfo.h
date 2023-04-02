#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QList>

#ifdef QTDROPBOX_DEBUG
#include <QtDebug>
#endif

#include "qdropbox2common.h"

//! Provides information and metadata about an entry in the Dropbox account
/*!
  This class is a more specialized version of QDropboxJson. It provides access to 
  the metadata of an entity (file or directory) stored on Dropbox.

  To obtain metadata information about any kind of entity stored on the Dropbox you
  have to use QDropbox::metadata() or QDropboxFile::metadata(). Those functions
  return an instance of this class that contains the required information. If an
  error occured while obtaining the metadata the functon isValid() will return
  <i>false</i>.
 */
class QDropbox2EntityInfo : public QObject
{
    Q_OBJECT

public:
    /*!
      Creates an empty instance of QDropbox2EntityInfo.
      \warning internal use only
      \param parent parent QObject
    */
    QDropbox2EntityInfo(QObject *parent = 0);

    /*!
      Creates an instance of QDropbox2EntityInfo based on the data provided
      in the JSON in string representation.

      \param jsonStr metadata JSON in string representation
      \param parent pointer to the parent QObject
    */
    QDropbox2EntityInfo(const QJsonObject& jsonData, QObject *parent = 0);

    /*!
       Creates a copy of an other QDropbox2EntityInfo instance.

       \param other original instance
     */
    QDropbox2EntityInfo(const QDropbox2EntityInfo &other);

    /*!
      Default destructor. Takes care of cleaning up when the object is destroyed.
    */
    ~QDropbox2EntityInfo();

    /*!
      Copies the values from an other QDropbox2EntityInfo instance to the
      current instance.

      \param other original instance
    */
    void copyFrom(const QDropbox2EntityInfo &other);

    /*!
      Works exactly like copyFrom() only as an operator.

      \param other original instance
    */
    QDropbox2EntityInfo& operator=(const QDropbox2EntityInfo& other);

    /*!
      Raw Dropbox file identifier.
    */
    QString   id()              const   { return _id; }

    /*!
      File size in bytes.
     */
    quint64   bytes()           const   { return _bytes; }

    /*!
      Bytes as a string, formatted for the locale.
    */
    QString   size()            const;

    /*!
      Timestamp of last modification on the server.
     */
    QDateTime serverModified()  const   { return _serverModified; }

    /*!
      Timestamp of desktop client upload.
     */
    QDateTime clientModified()  const   { return _clientModified; }

    /*!
      Full canonical path of the file.
    */
    QString   path()            const   { return _path; }

    /*!
      Filename component.
    */
    QString   filename()        const   { return _filename; }

    /*!
      Indicates whether the selected item is currently shared with others.
    */
    bool      isShared()        const   { return _isShared; }

    /*!
      Indicates whether the selected item is a directory.
    */
    bool      isDirectory()     const   { return _isDir; }

    /*!
      Indicates whether the item is deleted on the server.
    */
    bool      isDeleted()       const   { return _isDeleted; }

    /*!
      Current revision as hash string. Use this for e.g. change check.
    */
    QString   revisionHash()    const   { return _revisionHash; }

private:
    void        init(const QJsonObject& jsonData = QJsonObject());
    QDateTime   getTimestamp(QString value);

    QString     _id;
    QDateTime   _clientModified;
    QDateTime   _serverModified;
    QString     _revisionHash;
    quint64     _bytes;
    QString     _size;
    QString     _path;
    QString     _filename;
    bool        _isShared;
    bool        _isDir;
    bool        _isDeleted;
};
