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
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>

#include "ESIUrls.h"

#include "ESIOAuth2UnknownCharacterAuthorizationCodeFlow.h"

namespace Evernus
{
    ESIOAuth2UnknownCharacterAuthorizationCodeFlow::ESIOAuth2UnknownCharacterAuthorizationCodeFlow(const QString &clientIdentifier,
                                                                                                   const QString &clientSecret,
                                                                                                   QObject *parent)
        : ESIOAuth2AuthorizationCodeFlow{clientIdentifier, clientSecret, parent}
    {
        connect(this, &ESIOAuth2UnknownCharacterAuthorizationCodeFlow::granted, this, &ESIOAuth2UnknownCharacterAuthorizationCodeFlow::checkCharacter);
    }

    void ESIOAuth2UnknownCharacterAuthorizationCodeFlow::checkCharacter()
    {
        const auto reply = get(ESIUrls::verifyUrl);
        connect(reply, &QNetworkReply::finished, this, [=] {
            reply->deleteLater();

            if (Q_UNLIKELY(reply->error() != QNetworkReply::NoError))
            {
                resetStatus();
                emit error(reply->errorString(), {}, reply->url());
                return;
            }

            const auto data = reply->readAll();

            const auto doc = QJsonDocument::fromJson(data);
            const auto object = doc.object();

            qDebug() << "Checking the character for auth:" << doc;

            emit characterConfirmed(object[QStringLiteral("CharacterID")].toDouble(Character::invalidId));
        });
    }
}
