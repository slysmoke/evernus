#include "qdropbox2account.h"

QDropbox2User::QDropbox2User(QObject *parent)
    : QObject(parent)
{
}

QDropbox2User::QDropbox2User(const QJsonObject& jsonData, QObject *parent)
    : QObject(parent)
{
    init(jsonData);
}

QDropbox2User::QDropbox2User(const QDropbox2User& other)
    : QObject()
{
    copyFrom(other);
}

void QDropbox2User::init(const QJsonObject& jsonData)
{
    if(jsonData.isEmpty() ||
       !jsonData.contains("referral_link") ||
       !jsonData.contains("name")  ||
       !jsonData.contains("account_id") ||
       !jsonData.contains("country") ||
       !jsonData.contains("email"))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "json invalid 1" << endl;
#endif
        _valid = false;
    }
    else
    {
        _referralLink.setUrl(jsonData.value("referral_link").toString(), QUrl::StrictMode);

        QJsonObject name = jsonData.value("name").toObject();
        _displayName    = name.value("display_name").toString();
        _id             = name.value("account_id").toString();
        _country        = name.value("country").toString();
        _email          = name.value("email").toString();
        _emailVerified  = name.value("email_verified").toBool();
        _locale         = name.value("locale").toString();
        _isPaired       = name.value("is_paired").toBool();

        QJsonObject account_type = jsonData.value("account_type").toObject();
        _type           = account_type.value(".tag").toString();
        _isDisabled     = jsonData.value("disabled").toBool();

        if(jsonData.contains("profile_photo_url"))
            _profilePhoto.setUrl(jsonData.value("profile_photo_url").toString(), QUrl::StrictMode);

        _valid = true;
    }

#ifdef QTDROPBOX_DEBUG
    qDebug() << "== account data ==" << endl;
    qDebug() << "reflink: " << _referralLink << endl;
    qDebug() << "displayname: " << _displayName << endl;
    qDebug() << "account_id: " << _id << endl;
    qDebug() << "country: " << _country << endl;
    qDebug() << "email: " << _email << endl;
    qDebug() << "== account data end ==" << endl;
#endif
}

QUrl QDropbox2User::referralLink() const
{
    return _referralLink;
}

QString QDropbox2User::displayName()  const
{
    return _displayName;
}

QString QDropbox2User::id()  const
{
    return _id;
}

QString QDropbox2User::country()  const
{
    return _country;
}

QString QDropbox2User::email()  const
{
    return _email;
}

bool QDropbox2User::emailVerified() const
{
    return _emailVerified;
}

QString QDropbox2User::locale() const
{
    return _locale;
}

bool QDropbox2User::isPaired() const
{
    return _isPaired;
}

QString QDropbox2User::type() const
{
    return _type;
}

bool QDropbox2User::isDisabled() const
{
    return _isDisabled;
}

QUrl QDropbox2User::profilePhoto() const
{
    return _profilePhoto;
}

QDropbox2User &QDropbox2User::operator =(QDropbox2User &a)
{
    copyFrom(a);
    return *this;
}

void QDropbox2User::copyFrom(const QDropbox2User &other)
{
    this->setParent(other.parent());
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating account from account" << endl;
#endif
    _referralLink  = other._referralLink;
    _id            = other._id;
    _displayName   = other._displayName;
    _email         = other._email;
    _emailVerified = other._emailVerified;
    _locale        = other._locale;
    _country       = other._country;
    _type          = other._type;
    _isDisabled    = other._isDisabled;
    _profilePhoto  = other._profilePhoto;
}

//------------------------------------------------------

QDropbox2Usage::QDropbox2Usage(QObject *parent)
    : QObject(parent)
{
}

QDropbox2Usage::QDropbox2Usage(const QJsonObject& jsonData, QObject *parent)
    : QObject(parent)
{
    init(jsonData);
}

QDropbox2Usage::QDropbox2Usage(const QDropbox2Usage& other)
    : QObject()
{
    copyFrom(other);
}

void QDropbox2Usage::init(const QJsonObject& jsonData)
{
    if(jsonData.isEmpty() ||
       !jsonData.contains("used") ||
       !jsonData.contains("allocation"))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "json invalid 1" << endl;
#endif
        _valid = false;
    }
    else
    {
        _used           = jsonData.value("used").toString().toULongLong();

        QJsonObject allocation = jsonData.value("allocation").toObject();
        _allocated      = allocation.value("allocated").toString().toULongLong();
        _allocationType = allocation.value(".tag").toString();

        _valid = true;
    }
}

quint64 QDropbox2Usage::used() const
{
    return _used;
}

QString QDropbox2Usage::allocationType() const
{
    return _allocationType;
}

quint64 QDropbox2Usage::allocated() const
{
    return _allocated;
}

QDropbox2Usage &QDropbox2Usage::operator =(QDropbox2Usage &a)
{
    copyFrom(a);
    return *this;
}

void QDropbox2Usage::copyFrom(const QDropbox2Usage &other)
{
    this->setParent(other.parent());
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating account from account" << endl;
#endif
    _used           = other._used;
    _allocationType = other._allocationType;
    _allocated      = other._allocated;
}
