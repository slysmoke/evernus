#pragma once

#include <QObject>
#include <QUrl>

#include "qdropbox2common.h"

//! Stores information about a user account
/*!
  This class is used to store user account information retrieved by using
  QDropbox2::userInfo(). The stored data directly correspond to the
  Dropbox APIv2 request get_current_account.

  QDropbox2Account interprets given data based on a QDropboxJson. If the data
  could be interpreted and hence is valid the resulting object will be valid.
  If any error occurs while interpreting the data the resulting QDropboxAccount
  object will be invalid. This can checked by using isValid().

  See https://www.dropbox.com/developers/documentation/http/documentation#users-get_current_account for details.
 */
class QDropbox2User : public QObject
{
    Q_OBJECT
public:
    /*!
      Creates an empty instance of the object. It is automatically invalid
      and does not contain useful data.

      \param parent Parent QObject.
     */
    QDropbox2User(QObject *parent = 0);

    /*!
      This constructor creates an object based on the data contained in the
      given string that is in valid JSON format.

      \param jsonString JSON data in string representation
      \param parent Parent QObject.
     */
    QDropbox2User(const QJsonObject& jsonData, QObject *parent = 0);

    /*!
      Use this constructor to create a copy of an other QDropboxAccount.

      \param other Original QDropboxAccount
     */
    QDropbox2User(const QDropbox2User& other);

    /*!
      Indicates that the class instance contains valid data.
     */
    bool isValid() const;

    /*!
      Returns the referral link for the account.  This can be used to gain
      the account additional space if provided by new account signups.
     */
    QUrl referralLink() const;

    /*!
      Returns the display name of the account.
     */
    QString displayName() const;

    /*!
      Returns the Dropbox account ID.
     */
    QString id() const;

    /*!
      Returns the country the account is associated to.
     */
    QString country() const;

    /*!
      Returns the locale for the account.
     */
    QString locale() const;

    /*!
      Returns the E-Mail address the owner of the account uses.
     */
    QString email() const;

    /*!
      Returns if the email address the owner has been verified.
     */
    bool emailVerified() const;

    /*!
      Returns if a work account is paired to this personal account.
     */
    bool isPaired() const;

    /*!
      Returns the account type ("business", "pro", "basic", etc.)
     */
    QString type() const;

    /*!
      Returns if the account has been disabled.
     */
    bool isDisabled() const;

    /*!
      Returns a URL, if available, to a photo for the account profile.
     */
    QUrl profilePhoto() const;

    ///*!
    //  Returns the user's used quota in shared folders in bytes.
    // */
    //quint64  quotaShared()  const;

    ///*!
    //  Returns the user's total quota of allocated bytes.
    // */
    //quint64  quota()  const;

    ///*!
    //  Returns the user's quota outside of shared folders in bytes.
    // */
    //quint64  quotaNormal()  const;

    /*!
      Overloaded operator to copy a QDropboxAccount by using =. Internally
      copyFrom() is called.
     */
    QDropbox2User& operator =(QDropbox2User&);

    /*!
      This function is used to copy the data from an other QDropboxAccount.
     */
    void copyFrom(const QDropbox2User& a);

private:  
    bool    _valid;

    QUrl    _referralLink;
    QString _id;
    QString _displayName;
    QString _email;
    bool    _emailVerified;
    QString _country;
    QString _locale;
    bool    _isPaired;
    QString _type;
    bool    _isDisabled;
    QUrl    _profilePhoto;

    void    init(const QJsonObject& jsonData);
};

//! Stores information about account usage
/*!
  This class is used to store user account usage information retrieved by using
  QDropbox2::usageInfo(). The stored data directly correspond to the
  Dropbox APIv2 request get_space_usage.

  QDropbox2Account interprets given data based on a QDropboxJson. If the data
  could be interpreted and hence is valid the resulting object will be valid.
  If any error occurs while interpreting the data the resulting QDropboxAccount
  object will be invalid. This can checked by using isValid().

  See https://www.dropbox.com/developers/documentation/http/documentation#users-get_space_usage for details.
 */
class QDropbox2Usage : public QObject
{
    Q_OBJECT
public:
    /*!
      Creates an empty instance of the object. It is automatically invalid
      and does not contain useful data.

      \param parent Parent QObject.
     */
    QDropbox2Usage(QObject *parent = 0);

    /*!
      This constructor creates an object based on the data contained in the
      given string that is in valid JSON format.

      \param jsonString JSON data in string representation
      \param parent Parent QObject.
     */
    QDropbox2Usage(const QJsonObject& jsonData, QObject *parent = 0);

    /*!
      Use this constructor to create a copy of an other QDropboxAccount.

      \param other Original QDropboxAccount
     */
    QDropbox2Usage(const QDropbox2Usage& other);

    /*!
      Indicates that the class instance contains valid data.
     */
    bool isValid() const;

    /*!
      Returns the number of bytes currently in use on the account.
     */
    quint64 used() const;

    /*!
      The way the value returned by allocated() should be interpreted.
     */
    QString allocationType() const;

    /*!
      Returns the allocated amount for the account, interpreted based on
      the allocationType().
     */
    quint64 allocated() const;

    /*!
      Overloaded operator to copy a QDropboxAccount by using =. Internally
      copyFrom() is called.
     */
    QDropbox2Usage& operator =(QDropbox2Usage&);

    /*!
      This function is used to copy the data from an other QDropboxAccount.
     */
    void copyFrom(const QDropbox2Usage& a);

private:  
    bool    _valid;

    quint64 _used;
    quint64 _allocated;
    QString _allocationType;

    void    init(const QJsonObject& jsonData);
};
