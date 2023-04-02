#pragma once

#include <QtCore/QFile>

#include "qdropbox2common.h"

#include "qdropbox2.h"
#include "qdropbox2entity.h"
#include "qdropbox2entityinfo.h"

//! Allows access to files stored on Dropbox
/*!
  QDropbox2File allows you to access files that are stored on Dropbox. You can
  use this class as any QIODevice, very similar to the default QFile class. It is
  usable in connection with QTextStream and QDataStream to access the file contents.

  It is important to know that QDropbox2File buffers the content of the remote file
  locally when using open(). This means that the file content is not automatically
  updated if it changed on the Dropbox server which, in return, means that you may
  not always have the most current version of the file content.
*/
class QDropbox2File : public QIODevice, public IQDropbox2Entity
{
    Q_OBJECT

public:     // typedefs and enums
    typedef QList<QDropbox2EntityInfo> RevisionsList;

public:
    /*!
      Default constructor. Use setApi() and setFilename() to access Dropbox.

      \param parent Parent QObject
     */
    QDropbox2File(QObject* parent = 0);

    /*!
      Copy constructor.

      \param source Source QDropbox2File to copy from
     */
    QDropbox2File(const QDropbox2File& source);

    /*!
      Creates an instance of QDropbox2File that may connect to Dropbox if the passed
      QDropbox2 is already connected. Use setFilename() before you try to access any
      file.

      \param api Pointer to a QDropbox2 that is connected to an account.
      \param parent Parent QObject
     */
    QDropbox2File(QDropbox2* api, QObject* parent = 0);

    /*!
      Creates an instance of QDropbox2File that may access a file on Dropbox.

      \param filename Dropbox path of the file you want to access.
      \param api A QDropbox2 that is connected to an user account.
      \param parent Parent QObject
     */
    QDropbox2File(const QString& filename, QDropbox2* api, QObject* parent = 0);

    /*!
      This deconstructor cleans up on destruction of the object.
     */
    ~QDropbox2File();

    /*!
      If an error occurred you can access the last error code by using this function.
     */
    int error();

    /*!
      After an error occurred you'll get a description of the last error by using this
      function.
     */
    QString errorString();

    /*!
      QDropbox2File is currently implemented as sequential device. That will
      change in time.
     */
    bool isSequential() const;

    /*!
      Fetches the file content from the Dropbox server and buffers it locally. Depending
      on the OpenMode read or write access will be granted.

      \param mode The access mode of the file. Equivalent to QIODevice.
     */
    bool open(OpenMode mode);

    /*!
      Closes the file buffer. If the file was opened with QIODevice::WriteOnly (or
      QIODevice::ReadWrite) the file content buffer will be flushed and written to
      the file.
     */
    void close();

    /*!
      Sets the QDropbox2 instance that is used to access Dropbox.

      \param dropbox Pointer to the QDropbox2 object
     */
    void setApi(QDropbox2* dropbox);

    /*!
      Returns a pointer to the QDropbox2 instance that is used to connect to Dropbox.
     */
    QDropbox2* api() const { return _api; }

    /*!
      Set the name of the file you want to access. Remember to use correct Dropbox path
      beginning with either /dropbox/ or /sandbox/.

      \param filename Path of the file.
     */
    void setFilename(const QString& filename);

    /*!
      Returns the path of the file that is accessed by this instance.
     */
    QString filename() const  { return _filename; }

    /*!
      Writes the content of the buffer to the file (only if the file is opened in
      write mode).
     */
    bool flush();

    /*!
      Reimplemented from QIODEvice.
     */
    bool event(QEvent* event);

    /*!
      Usually the file content is automatically flushed whenever the internal buffer
      has more than 1024 new byte or on using close(). If you want QDropbox2File to
      automatically flush earlier than those 1024 byte use this function to reduce
      this threshold.

      \param num QDropbox2File will automatically flush the file buffer when there are
                 more than num new byte of data.
     */
    //void setFlushThreshold(qint64 num);

    /*!
      Returns the current flush threshold setting.
     */
    //qint64 flushThreshold() const { return bufferThreshold; }

    /*!
      By default an already existing file will be overwritten. If you don't want to
      let this happen use this function to set the overwrite flag to false. If a file
      with the same name already exists it will be automatically renamed by Dropbox to
      something like "file (1).txt".

      \param overwrite Overwrite flag
     */
    void setOverwrite(bool overwrite = true);

    /*!
      Returns the current state of the overwrite flag.
     */
    bool overwrite() const { return overwrite_; }

    /*!
      Specifies the behavior Dropbox should take when it encounters a file with the same
      name on uploading.  The semantics of renaming have impact even when overwriting is
      true, so be aware of the implications.

      \param rename Renaming flag
     */
    void setRenaming(bool rename = true);

    /*!
      Returns the current state of the renaming flag.
     */
    bool renaming() const { return rename; }

    /*!
      Return the metadata of the file as a QDropbox2EntityInfo object.
    */
    QDropbox2EntityInfo metadata();

    /*!
      Retrieves a (temporary) URL link to the file for streaming.  The file
      type must support streaming (e.g., MP4) for the link to work properly.

      \remark The link is only valid for 4 hours.

      \remark This is a blocking call.

      \returns A <i>valid</i> QUrl suitable for use with QDesktopServices::openUrl(), or an <i>invalid</i> QUrl on failure.
    */
    QUrl temporaryLink();

    /*!
      Remove the file from Dropbox.

      \remark The option to remove the file permanently is only
      available to business endpoints.

      \remark This is a blocking call.

      \returns <i>true</i> if the file was successfully removed or <i>false</i> if there was an error.
    */
    bool remove(bool permanently = false);

    /*!
      Move the file to a new location in the Dropbox account.

      \remark This is a blocking call.

      \returns <i>true</i> if the file was successfully moved or <i>false</i> if there was an error.
    */
    bool move(const QString& to_path);

    /*!
      Copy the contents of a file to a new location in the
      Dropbox account.

      \remark This is a blocking call.

      \returns <i>true</i> if the file was successfully copied or <i>false</i> if there was an error.
    */
    bool copy(const QString& to_path);

    /*!
      Check if the file has changed on the dropbox while it was opened locally.
      This function will return false if the file was not previously opened and an error
      occurred during the retrieval of the file metadata. Hence it is safer to open the file
      first and then check hasChanged()

      \remark This is a blocking call.

      \returns <i>true</i> if the file has changed or <i>false</i> if it has not.
    */
    bool hasChanged();

    /*!
      Retrieves available revisions of the file up to a maximum.

      \remark This is a blocking call.

      \param revisions Container to receive the returned list of revisions.
      \param max_results The function will only return up to the specified number of revisions.
      \returns <i>true</i> revisions were retrieved or <i>false</i> if there was an error.
    */
    bool revisions(RevisionsList& revisions, quint64 max_results = 10);

    /*!
      Retrieves available revisions of the file up to a maximum.

      \remark This is an asynchronous call. Emits a signal when revisions
      results have been retrieved.

      \param max_results The function will only return up to the specified number of revisions.
      \returns <i>true</i> if the revision request was successfully submitted or <i>false</i> if there was an error.
    */
    bool revisions(quint64 max_results = 10);

    /*!
      Reimplemented from QIODevice::seek().
      Foreward to the given (byte) position in the file. Unlike QFile::seek() this function does
      not seek beyond the file end. When seeking beyond the end of a file this function stops beyond
      the last byte of the current content and returns <code>false</code>.
    */
    bool seek(qint64 pos);

    /*!
      Reimplemented from QIODevice::pos().
      Returns the current position in the file.
    */
    qint64 pos() const  { return position; }

    /*!
      Reimplemented from QIODevice::reset().
      Seeks to the beginning of the file. See seek().
    */
    bool reset();

    /*!
      Reimplemented from QIODevice::bytesAvailable().
      Reports the current size of the data buffer.
    */
    virtual qint64 bytesAvailable() const;

    /*!
      Reimplemented from IQDropbox2Entity.
    */
    virtual bool isDir() const { return false; }

public slots:
    void    slot_abort();

signals:
    /*!
      This signal is emitted whenever an error occurs. The error is passed
      as parameter to the slot. To retrieve descriptive information about
      the error use errorString().

      \param errorcode The occurred error.
      \param errormessage A text string version of the error, if available.
     */
    void    signal_errorOccurred(int errorcode, const QString& errormessage = QString());

    /*!
      Emitted as file contents are downloaded from Dropbox.

      \param bytesReceived The amount of data received so far.
      \param bytesTotal Total expected size of the payload.
     */
    void    signal_downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    /*!
      Emitted as file contents are uploaded to Dropbox.

      \param bytesSent The amount of data sent so far.
      \param bytesTotal Total size of the outgoing payload.
     */
    void    signal_uploadProgress(qint64 bytesSent, qint64 bytesTotal);

    /*!
      This signal is emitted when an in-progress operation is aborted.
     */
    void    signal_operationAborted();

    /*!
      Emitted when the requested list of file revisions has been processed.

      \param reply The QNetworkReply for the request.
     */
    void    signal_revisionsResult(const RevisionsList& revisions);

protected:
    // QIODevice reimplemented methods
    qint64  readData(char *data, qint64 maxlen);
    qint64  writeData(const char *data, qint64 len);

private slots:
    void    slot_networkRequestFinished(QNetworkReply* rply);
    void    slot_uploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:        // typedefs and enums
    struct CallbackData;
    typedef QSharedPointer<CallbackData> CallbackPtr;
    typedef QMap<QNetworkReply*, CallbackPtr> ReplyMap;

    struct SessionData;
    typedef QSharedPointer<SessionData> SessionPtr;
    typedef QMap<QNetworkReply*, SessionPtr> SessionMap;
    typedef QMap<QNetworkReply*, QString> SessionStartMap;

    typedef void(QDropbox2File::*AsyncCallback)(QNetworkReply*, CallbackPtr);

private:        // classes
    // since we have simple types, and since we won't create instances
    // without explicitly setting their values, we opt for the default
    // constructors on this class to reduce code clutter.
    struct CallbackData
    {
        AsyncCallback callback;
    };

    // this data contains per-session settings for the "upload_session"
    // action.  it's possible to have multiple upload sessions running
    // concurrently, and this is a step in that direction.  however, for
    // now, we will only do one session at a time circumscribed to each
    // QDropbox2File class.  for efficiency, I'm thinking this could be
    // pulled out to some kind of external upload-session manager
    // instance at a later time with which each QDropbox2File upload
    // will register.
    struct SessionData
    {
        QString     session_parameters;
        QString     session_id;
        int         session_offset;
        int         session_payload;

        SessionData()
            : session_offset(0),
              session_payload(0)
        {}
    };

private:        // methods
    void    init(QDropbox2 *api, const QString& filename, qint64 threshold = MaxSingleUpload);

    QNetworkReply* sendPOST(QNetworkRequest& rq, QByteArray& postdata);
    QNetworkReply* sendGET(QNetworkRequest& rq);

    bool    isMode(QIODevice::OpenMode mode);
    bool    getFile(const QString& filename);
    bool    putFile();
    void    obtainMetadata();

    bool    requestRemoval(bool permanently);
    bool    requestMove(const QString& to_path);
    bool    requestCopy(const QString& to_path);
    QUrl    requestStreamingLink();

    // Note that the QNetworkReply pointer is returned in case the
    // function needs to set data into the AsyncMap for later access
    // by the callback function.
    bool    getRevisions(QNetworkReply*& reply, quint64 max_results, bool async = false);

    // functions for synchronous actions
    void    startEventLoop();
    void    stopEventLoop();

    // QNetworkReply post-processing callbacks (synchronous and asynchronous)
    void    resultGetFile(QNetworkReply* reply, CallbackPtr reply_data);
    void    resultPutFile(QNetworkReply* reply, CallbackPtr reply_data);
    void    revisionsCallback(QNetworkReply* reply, CallbackPtr reply_data);

private:        // data members
    QNetworkAccessManager QNAM;

    QByteArray  *_buffer;

    QString     accessToken;
    QString     _filename;

    QDropbox2   *_api;

    // for deferred functions
    ReplyMap    replyMap;

    // for synchronous functions
    QEventLoop* eventLoop;

    int         lastErrorCode;
    QString     lastErrorMessage;
    QString     lastResponse;

    QString     lastHash;

    bool        fileExists;

    qint64      bufferThreshold;
    qint64      currentThreshold;

    bool        overwrite_;
    bool        rename;

    int         position;

    // for upload_session
    SessionStartMap session_starts;
    SessionMap  upload_sessions;

    QDropbox2EntityInfo *_metadata;
};

Q_DECLARE_METATYPE(QDropbox2File);
