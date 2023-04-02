#pragma once

#include "qdropbox2common.h"

#include "qdropbox2.h"
#include "qdropbox2entity.h"
#include "qdropbox2entityinfo.h"

//! Allows access to folders stored on Dropbox

class QDropbox2Folder : public QObject, public IQDropbox2Entity
{
    Q_OBJECT

public:     // typedefs and enums
    typedef QList<QDropbox2EntityInfo> ContentsList;

public:
    /*!
      Default constructor. Use setApi() and setFilename() to access Dropbox.

      \param parent Parent QObject
     */
    QDropbox2Folder(QObject* parent = 0);

    /*!
      Copy constructor.

      \param source Source QDropbox2Folder to copy from
     */
    QDropbox2Folder(const QDropbox2Folder& source);

    /*!
      Creates an instance of QDropbox2Folder that may connect to Dropbox if the passed
      QDropbox2 is already connected. Use setFoldername() before you try to access any
      folder.

      \param api Pointer to a QDropbox2 that is connected to an account.
      \param parent Parent QObject
     */
    QDropbox2Folder(QDropbox2* api, QObject* parent = 0);

    /*!
      Creates an instance of QDropbox2File that may access a file on Dropbox.

      \param foldername Dropbox path of the folder you want to access.
      \param api A QDropbox2 that is connected to an user account.
      \param parent Parent QObject
     */
    QDropbox2Folder(const QString& foldername, QDropbox2* api, QObject* parent = 0);

    /*!
      This deconstructor cleans up on destruction of the object.
     */
    ~QDropbox2Folder();

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
    void setFoldername(const QString& foldername);

    /*!
      Returns the path of the file that is accessed by this instance.
     */
    QString foldername() const  { return _foldername; }

    /*!
      Specifies the behavior Dropbox should take when it encounters a folder with the same
      name on creation.

      \param rename Renaming flag
     */
    void setRenaming(bool rename = true);

    /*!
      Returns the current state of the renaming flag.
     */
    bool renaming() const { return rename; }

    /*!
      Return the metadata for the folder as a QDropbox2EntityInfo object.
    */
    QDropbox2EntityInfo metadata();

    /*!
      Create the folder in Dropbox.

      \remark This protocol will automatically create all path elements if
      they do not already exist.  There is no need to make QDropbox2Folder
      instances, or re-target a single instance multiple times, to create a
      full directory chain.

      \remark This is a blocking call.

      \returns <i>true</i> if the folder (chain) was created or already exists; <i>false</i> if there was an error.
    */
    bool create();

    /*!
      Remove the folder from Dropbox.

      \remark The option to remove the file permanently is only
      available to business endpoints.

      \remark This is a blocking call.

      \returns <i>true</i> if the folder was removed or <i>false</i> if there was an error.
    */
    bool remove(bool permanently = false);

    /*!
      Move the folder to a new location in the Dropbox account.

      \remark This is a blocking call.

      \returns <i>true</i> if the folder was moved or <i>false</i> if there was an error.
    */
    bool move(const QString& to_path);

    /*!
      Copy the contents of a folder to a new location in the
      Dropbox account.

      \remark This is a blocking call.

      \returns <i>true</i> if the folder was copied or <i>false</i> if there was an error.
    */
    bool copy(const QString& to_path);

    /*!
      Use this to poll a folder contents for changes since the last check.
      The persistent cursor value will be reset on each call.

      \remark This is a blocking call.

      \param changes Container to receive the changes detected.
      \returns <i>true</i> if the folder has changes or <i>false</i> if it has not.
    */
    bool hasChanged(ContentsList& changes);

    /*!
      Poll a folder contents for changes since the last check.

      \remark This is an asynchronous call. Emits a signal when changes have
      been retrieved.

      \returns <i>true</i> if the request was submitted successfully or <i>false</i> if it was not.
    */
    bool hasChanged();

    /*!
      Wait for a change to occur in the folder based on the currently cached cursor value,
      or until a specified timeout has expired.  If changes are detected, you can call
      hasChanged() to retrieve them.

      \remark This function will block the calling thread until a change is detected
      or the timeout expires.

      \param timeout The number of seconds to wait.  According to the APIv2 documentation, this delay may only be vaguely accurate.
      \returns <i>true</i> if a change was detected or <i>false</i> if the timeout expired.
    */
    bool waitForChanged(int timeout = 30);

    /*!
      Gets and returns all the contents of the folder.

      You can drill down into the folder tree by identifying folders in
      the returned contents, and then issuing a contents() call on each.

      \remark This is a blocking call.

      \param contents Container to receive the list results.
      \param include_folders Include folders in the result.
      \param include_deleted Include deleted files in the result.
      \returns <i>true</i> if the retreival was successful or <i>false</i> if it was not.
    */
    bool contents(ContentsList& contents, bool include_folders = true, bool include_deleted = false);

    /*!
      Gets and returns all the contents of the folder.

      You can drill down into the folder tree by identifying folders in
      the returned contents, and then issuing a contents() call on each.

      \remark This is an asynchronous call. Emits a signal when contents have
      been retrieved.

      \param include_folders Include folders in the result.
      \param include_deleted Include deleted files in the result.
      \returns <i>true</i> if the retreival was successful or <i>false</i> if it was not.
    */
    bool contents(bool include_folders = true, bool include_deleted = false);

    /*!
      Search for files and folders that match the search query.

      \remark The content-based mode is only available to business endpoints.

      \remark This is a blocking call.

      \param contents Container to receive the search results.
      \param query The query string to match against entries.
      \param max_results The maximum number of results to return.
      \param mode The search mode, one of 'filename', 'filename_and_content' or 'filename_deleted'.
      \returns <i>true</i> if the folder was copied or <i>false</i> if there was an error.
    */
    bool search(ContentsList& contents, const QString& query, quint64 max_results = 100, const QString& mode = "filename");

    /*!
      Search for files and folders that match the search query.

      \remark The content-based mode is only available to business endpoints.

      \remark This is an asynchronous call. Emits a signal when search results
      have been retrieved.

      \param query The query string to match against entries.
      \param max_results The maximum number of results to return.
      \param mode The search mode, one of 'filename', 'filename_and_content' or 'filename_deleted'.
      \returns <i>true</i> if the folder was copied or <i>false</i> if there was an error.
    */
    bool search(const QString& query, quint64 max_results = 100, const QString& mode = "filename");

    /*!
      Reimplemented from IQDropbox2Entity.
    */
    virtual bool isDir() const { return true; }

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

    void    signal_operationAborted();

    void    signal_contentsResults(const ContentsList& contents_results);
    void    signal_searchResults(const ContentsList& search_results);
    void    signal_hasChangedResults(const ContentsList& change_results);

private slots:
    void    slot_networkRequestFinished(QNetworkReply* rply);

private:        // typedefs and enums
    struct CallbackData;
    typedef QSharedPointer<CallbackData> CallbackPtr;
    typedef QMap<QNetworkReply*, CallbackPtr> ReplyMap;

    typedef void(QDropbox2Folder::*AsyncCallback)(QNetworkReply*, CallbackPtr);

private:        // classes
    // since we have simple types, and since we won't create instances
    // without explicitly setting their values, we opt for the default
    // constructors on these classes to reduce code clutter.
    struct CallbackData
    {
        AsyncCallback callback;
    };
    struct ContentsData : public CallbackData
    {
        bool include_folders;
    };

private:        // methods
    void    init(QDropbox2 *api, const QString& foldername);

    QNetworkReply* sendPOST(QNetworkRequest& rq, QByteArray& postdata);

    void    obtainMetadata();

    bool    requestCreation();
    bool    requestRemoval(bool permanently);
    bool    requestMove(const QString& to_path);
    bool    requestCopy(const QString& to_path);
    bool    requestLongpoll(int timeout = 30);

    bool    getLatestCursor(QString& cursor, bool include_deleted = true);

    // Note that the QNetworkReply pointer is returned in case the
    // function needs to set data into the AsyncMap for later access
    // by the callback function.
    bool    getContents(QNetworkReply*& reply, const QString& cursor, bool include_deleted = false, bool async = false);
    bool    getSearch(QNetworkReply*& reply, const QString& query, quint64 start, quint64 max_results, const QString& mode, bool async);

    // functions for synchronous actions
    void    startEventLoop();
    void    stopEventLoop();

    // QNetworkReply post-processing callbacks (synchronous and asynchronous)
    void    contentsCallback(QNetworkReply* reply, CallbackPtr data);
    void    searchCallback(QNetworkReply* reply, CallbackPtr data);
    void    hasChangedCallback(QNetworkReply* reply, CallbackPtr data);

private:        // data members
    QNetworkAccessManager QNAM;

    QString     accessToken;
    QString     _foldername;

    QDropbox2   *_api;

    // for asynchronous operations
    ReplyMap    replyMap;

    // for synchronous operations
    QEventLoop* eventLoop;

    int         lastErrorCode;
    QString     lastErrorMessage;
    QString     lastResponse;

    bool        overwrite_;
    bool        rename;

    QString     latestCursor;

    QDropbox2EntityInfo *_metadata;
};

Q_DECLARE_METATYPE(QDropbox2Folder);
