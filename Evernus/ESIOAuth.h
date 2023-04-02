/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <unordered_map>
#include <functional>
#include <vector>

#include <QAbstractOAuth>
#include <QVariant>
#include <QList>

#include "ESINetworkAccessManager.h"
#include "SimpleCrypt.h"
#include "Character.h"

class QByteArray;
class QSslError;

namespace Evernus
{
    class ESIOAuth2CharacterAuthorizationCodeFlow;
    class CharacterRepository;
    class EveDataProvider;

    class ESIOAuth final
        : public QObject
    {
        Q_OBJECT

    public:
        using NetworkReplyCallback = std::function<void (QNetworkReply &)>;
        using AuthErrorCallback = std::function<void (const QString &)>;

        ESIOAuth(QString clientId,
                 QString clientSecret,
                 const CharacterRepository &characterRepo,
                 const EveDataProvider &dataProvider,
                 QObject *parent = nullptr);
        ESIOAuth(const ESIOAuth &) = default;
        ESIOAuth(ESIOAuth &&) = default;
        virtual ~ESIOAuth() = default;

        void get(Character::IdType charId, const QUrl &url, QVariantMap parameters, NetworkReplyCallback callback, AuthErrorCallback errorCallback);
        QNetworkReply *get(QUrl url, QVariantMap parameters = {});
        QNetworkReply *post(QUrl url, const QVariant &data = {});
        void post(Character::IdType charId, QUrl url, const QVariant &data, NetworkReplyCallback callback, AuthErrorCallback errorCallback);

        void clearRefreshTokens();

        void processSSOAuthorizationCode(Character::IdType charId, const QByteArray &rawQuery);
        void cancelSsoAuth(Character::IdType charId);
        void setTokens(Character::IdType id, const QString &accessToken, const QString &refreshToken);

        static QString getUserAgent();

        ESIOAuth &operator =(const ESIOAuth &) = default;
        ESIOAuth &operator =(ESIOAuth &&) = default;

    signals:
        void ssoAuthRequested(Character::IdType charId, const QUrl &url);
        void ssoAuthReceived(Character::IdType charId, const QVariantMap &data);

    private slots:
        void processSslErrors(const QList<QSslError> &errors);

    private:
        struct PendingCallbacks
        {
            std::function<void ()> mRequestCallback;
            AuthErrorCallback mErrorCallback;

            PendingCallbacks(std::function<void ()> requestCallback, AuthErrorCallback errorCallback);
        };

        QString mClientId;
        QString mClientSecret;

        const CharacterRepository &mCharacterRepo;
        const EveDataProvider &mDataProvider;

        SimpleCrypt mCrypt;

        std::unordered_map<Character::IdType, ESIOAuth2CharacterAuthorizationCodeFlow *> mCharactersOAuths;
        std::unordered_map<Character::IdType, QString> mRefreshTokens;

        ESINetworkAccessManager mUnauthNetworkAccessManager;

        std::unordered_map<Character::IdType, std::vector<PendingCallbacks>> mPendingRequests;

        ESIOAuth2CharacterAuthorizationCodeFlow &getOAuth(Character::IdType charId);

        void prepareParameters(QVariantMap &parameters);
        void prepareUrl(QUrl &url);

        template<class T>
        void makeRequest(Character::IdType charId, const QUrl &url, NetworkReplyCallback callback, AuthErrorCallback errorCallback, T replyCreator);
        template<class T>
        void queueRequest(Character::IdType charId, const QUrl &url, NetworkReplyCallback callback, AuthErrorCallback errorCallback, T replyCreator);

        void processPendingRequests(Character::IdType charId);
        void processPendingRequests(Character::IdType charId, const QString &error);
        void resetOAuthStatus(Character::IdType charId) const;

        void saveRefreshToken(Character::IdType charId);

        static QNetworkRequest prepareRequest(const QUrl &url);

        static void grantOrRefresh(ESIOAuth2CharacterAuthorizationCodeFlow &oauth);
    };
}
