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

#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

#include "ESIUrls.h"

#include "ESIOAuthReplyHandler.h"

namespace Evernus
{
    ESIOAuthReplyHandler::ESIOAuthReplyHandler(Character::IdType charId, QString scope, QObject *parent)
        : QAbstractOAuthReplyHandler{parent}
        , mCharId{charId}
        , mScope{std::move(scope)}
    {
    }

    QString ESIOAuthReplyHandler::callback() const
    {
        return ESIUrls::callbackUrl;
    }

    void ESIOAuthReplyHandler::networkReplyFinished(QNetworkReply *reply)
    {
        Q_ASSERT(reply != nullptr);

        const auto data = reply->readAll();
        const auto replyError = reply->error();

        if (Q_UNLIKELY(replyError != QNetworkReply::NoError))
        {
            const auto errorObj = QJsonDocument::fromJson(data).object();
            const auto errorStr = (errorObj.contains(QStringLiteral("error"))) ? (errorObj.value(QStringLiteral("error")).toString()) : (reply->errorString());

            qWarning() << "OAuth error:" << replyError << errorStr << data;

            emit error(errorStr);
            emit tokensReceived({{ QStringLiteral("error"), errorStr }});
            return;
        }

        emit replyDataReceived(data);

        const auto doc = QJsonDocument::fromJson(data);
        auto params = doc.object().toVariantMap();

        // https://bugreports.qt.io/browse/QTBUG-66415
        // TODO: remove when fixed
        if (!params.contains(QStringLiteral("scope")))
            params[QStringLiteral("scope")] = mScope;

        qDebug() << "ESI reply handler params:" << params;

        emit tokensReceived(params);
    }

    void ESIOAuthReplyHandler::handleAuthReply(Character::IdType charId, const QVariantMap &data)
    {
        if (charId != mCharId)
            return;

        emit callbackReceived(data);
    }
}
