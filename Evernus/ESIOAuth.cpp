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
#include <QtDebug>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QSettings>
#include <QUrlQuery>
#include <QUrl>

#include "ESIOAuth2CharacterAuthorizationCodeFlow.h"
#include "ESIOAuthReplyHandler.h"
#include "SecurityHelper.h"
#include "SSOSettings.h"
#include "SSOUtils.h"

#include "ESIOAuth.h"

namespace Evernus
{
    ESIOAuth::PendingCallbacks::PendingCallbacks(std::function<void ()> requestCallback, AuthErrorCallback errorCallback)
        : mRequestCallback{std::move(requestCallback)}
        , mErrorCallback{std::move(errorCallback)}
    {
    }

    ESIOAuth::ESIOAuth(QString clientId,
                       QString clientSecret,
                       const CharacterRepository &characterRepo,
                       const EveDataProvider &dataProvider,
                       QObject *parent)
        : QObject{parent}
        , mClientId{std::move(clientId)}
        , mClientSecret{std::move(clientSecret)}
        , mCharacterRepo{characterRepo}
        , mDataProvider{dataProvider}
        , mCrypt{SSOSettings::cryptKey}
    {
        QSettings settings;
        settings.beginGroup(SSOSettings::refreshTokenGroup);

        const auto keys = settings.childKeys();
        for (const auto &key : keys)
            mRefreshTokens[key.toULongLong()] = mCrypt.decryptToString(settings.value(key).toByteArray());

        settings.endGroup();
    }

    void ESIOAuth::get(Character::IdType charId, const QUrl &url, QVariantMap parameters, NetworkReplyCallback callback, AuthErrorCallback errorCallback)
    {
        prepareParameters(parameters);
        makeRequest(charId, url, std::move(callback), std::move(errorCallback), [=, parameters = std::move(parameters)] {
            return getOAuth(charId).get(url, parameters);
        });
    }

    QNetworkReply *ESIOAuth::get(QUrl url, QVariantMap parameters)
    {
        prepareParameters(parameters);

        QUrlQuery query{url.query()};
        for (auto param = std::begin(parameters); param != std::end(parameters); ++param)
            query.addQueryItem(param.key(), param.value().toString());

        url.setQuery(query);

        const auto reply = mUnauthNetworkAccessManager.get(prepareRequest(url));
        connect(reply, &QNetworkReply::sslErrors, this, &ESIOAuth::processSslErrors);

        return reply;
    }

    QNetworkReply *ESIOAuth::post(QUrl url, const QVariant &data)
    {
        prepareUrl(url);

        auto request = prepareRequest(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

        const auto reply = mUnauthNetworkAccessManager.post(request, QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact));
        connect(reply, &QNetworkReply::sslErrors, this, &ESIOAuth::processSslErrors);

        return reply;
    }

    void ESIOAuth::clearRefreshTokens()
    {
        mRefreshTokens.clear();
        for (const auto &oauth : mCharactersOAuths)
            oauth.second->setRefreshToken({});
    }

    void ESIOAuth::processSSOAuthorizationCode(Character::IdType charId, const QByteArray &rawQuery)
    {
        emit ssoAuthReceived(charId, SSOUtils::parseAuthorizationCode(rawQuery));
    }

    void ESIOAuth::cancelSsoAuth(Character::IdType charId)
    {
        processPendingRequests(charId, tr("Authentication cancelled."));
    }

    void ESIOAuth::setTokens(Character::IdType id, const QString &accessToken, const QString &refreshToken)
    {
        const auto auth = mCharactersOAuths.find(id);
        if (auth != std::end(mCharactersOAuths))
        {
            auth->second->setToken(accessToken);
            auth->second->setRefreshToken(refreshToken);
        }

        mRefreshTokens[id] = refreshToken;
    }

    void ESIOAuth::post(Character::IdType charId, QUrl url, const QVariant &data, NetworkReplyCallback callback, AuthErrorCallback errorCallback)
    {
        prepareUrl(url);
        makeRequest(charId, url, std::move(callback), std::move(errorCallback), [=] {
            const auto &oauth = getOAuth(charId);

            // done manually because https://bugreports.qt.io/browse/QTBUG-65558

            QNetworkRequest request{url};
            request.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
            request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(oauth.token()).toUtf8());

            return oauth.networkAccessManager()->post(request, QJsonDocument::fromVariant(data).toJson());
        });
    }

    void ESIOAuth::processSslErrors(const QList<QSslError> &errors)
    {
        SecurityHelper::handleSslErrors(errors, *qobject_cast<QNetworkReply *>(sender()));
    }

    ESIOAuth2CharacterAuthorizationCodeFlow &ESIOAuth::getOAuth(Character::IdType charId)
    {
        auto it = mCharactersOAuths.find(charId);
        if (it == std::end(mCharactersOAuths))
        {
            it = mCharactersOAuths.emplace(charId, new ESIOAuth2CharacterAuthorizationCodeFlow{
                charId,
                mCharacterRepo,
                mDataProvider,
                mClientId,
                mClientSecret,
                this
            }).first;
            it->second->setRefreshToken(mRefreshTokens[charId]);

            const auto replyHandler = new ESIOAuthReplyHandler{charId, it->second->scope(), it->second};
            it->second->setReplyHandler(replyHandler);
            connect(replyHandler, &ESIOAuthReplyHandler::error, this, [=](const auto &error) {
                processPendingRequests(charId, error);
                resetOAuthStatus(charId);
            });
            connect(this, &ESIOAuth::ssoAuthReceived, replyHandler, &ESIOAuthReplyHandler::handleAuthReply);

            connect(it->second, &ESIOAuth2CharacterAuthorizationCodeFlow::authorizeWithBrowser,
                    this, [=](const auto &url) {
                emit ssoAuthRequested(charId, url);
            });
            connect(it->second, &ESIOAuth2CharacterAuthorizationCodeFlow::characterConfirmed,
                    this, [=] {
                saveRefreshToken(charId);
                processPendingRequests(charId);
            });
            connect(it->second, &ESIOAuth2CharacterAuthorizationCodeFlow::error, this, [=](const auto &error, const auto &description, const auto &url) {
                Q_UNUSED(description);
                Q_UNUSED(url);

                processPendingRequests(charId, error);
                resetOAuthStatus(charId);
            });
        }

        return *it->second;
    }

    void ESIOAuth::prepareParameters(QVariantMap &parameters)
    {
#ifdef EVERNUS_ESI_SISI
        parameters[QStringLiteral("datasource")] = QStringLiteral("singularity");
#else
        parameters[QStringLiteral("datasource")] = QStringLiteral("tranquility");
#endif
    }

    void ESIOAuth::prepareUrl(QUrl &url)
    {
        QUrlQuery query{url.query()};
#ifdef EVERNUS_ESI_SISI
        query.addQueryItem(QStringLiteral("datasource"), QStringLiteral("singularity"));
#else
        query.addQueryItem(QStringLiteral("datasource"), QStringLiteral("tranquility"));
#endif

        url.setQuery(query);
    }

    template<class T>
    void ESIOAuth::makeRequest(Character::IdType charId, const QUrl &url, NetworkReplyCallback callback, AuthErrorCallback errorCallback, T replyCreator)
    {
        auto &auth = getOAuth(charId);
        const auto status = auth.status();

        qDebug() << "ESI OAuth:" << charId << url << static_cast<int>(status);

        if (status == QAbstractOAuth::Status::Granted)
        {
            const auto reply = replyCreator();
            Q_ASSERT(reply != nullptr);

            connect(reply, &QNetworkReply::sslErrors, this, &ESIOAuth::processSslErrors);
            connect(reply, &QNetworkReply::finished, this, [=, &auth, callback = std::move(callback), errorCallback = std::move(errorCallback)] {
                reply->deleteLater();

                const auto error = reply->error();

                qDebug() << "ESI request:" << url << error;

                if (error == QNetworkReply::AuthenticationRequiredError || error == QNetworkReply::ContentAccessDenied)
                {
                    const auto expiration = auth.expirationAt();
                    qDebug() << "Token expiration:" << expiration;

                    if (QDateTime::currentDateTime().secsTo(expiration) < 5)    // allow 5s window threshold for updating the token
                    {
                        queueRequest(charId, url, std::move(callback), std::move(errorCallback), std::move(replyCreator));
                        if (mPendingRequests[charId].size() == 1)
                            grantOrRefresh(auth);
                    }
                    else
                    {
                        callback(*reply);
                    }
                }
                else
                {
                    callback(*reply);
                }
            });
        }
        else
        {
            queueRequest(charId, url, std::move(callback), std::move(errorCallback), std::move(replyCreator));

            if (status == QAbstractOAuth::Status::NotAuthenticated)
                grantOrRefresh(auth);
        }
    }

    template<class T>
    void ESIOAuth::queueRequest(Character::IdType charId, const QUrl &url, NetworkReplyCallback callback, AuthErrorCallback errorCallback, T replyCreator)
    {
        mPendingRequests[charId].emplace_back([=, callback = std::move(callback), replyCreator = std::move(replyCreator)] {
            makeRequest(charId, url, std::move(callback), std::move(errorCallback), replyCreator);
        }, errorCallback);
    }

    void ESIOAuth::processPendingRequests(Character::IdType charId)
    {
        const auto requests = std::move(mPendingRequests[charId]);
        for (const auto &request : requests)
            request.mRequestCallback();
    }

    void ESIOAuth::processPendingRequests(Character::IdType charId, const QString &error)
    {
        const auto requests = std::move(mPendingRequests[charId]);
        for (const auto &request : requests)
            request.mErrorCallback(error);
    }

    void ESIOAuth::resetOAuthStatus(Character::IdType charId) const
    {
        // TODO: remove when https://bugreports.qt.io/browse/QTBUG-66097 is fixed
        const auto oauth = mCharactersOAuths.find(charId);
        if (Q_LIKELY(oauth != std::end(mCharactersOAuths)))
            oauth->second->resetStatus();
    }

    void ESIOAuth::saveRefreshToken(Character::IdType charId)
    {
        const auto oauth = mCharactersOAuths.find(charId);
        if (Q_LIKELY(oauth != std::end(mCharactersOAuths)))
        {
            const auto token = oauth->second->refreshToken();

            mRefreshTokens[charId] = token;

            QSettings settings;
            settings.beginGroup(SSOSettings::refreshTokenGroup);
            settings.setValue(QString::number(charId), mCrypt.encryptToByteArray(token));
            settings.endGroup();
        }
    }

    QNetworkRequest ESIOAuth::prepareRequest(const QUrl &url)
    {
        QNetworkRequest request{url};
        request.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());

        return request;
    }

    QString ESIOAuth::getUserAgent()
    {
        return QStringLiteral("%1 %2").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
    }

    void ESIOAuth::grantOrRefresh(ESIOAuth2CharacterAuthorizationCodeFlow &oauth)
    {
        if (oauth.refreshToken().isEmpty())
            oauth.grant();
        else
            oauth.refreshAccessToken();
    }
}
