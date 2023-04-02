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

#include <QAbstractOAuthReplyHandler>
#include <QObject>

#include "ESIOAuth2UnknownCharacterAuthorizationCodeFlow.h"
#include "Character.h"

namespace Evernus
{
    class NewCharacterController final
        : public QAbstractOAuthReplyHandler
    {
        Q_OBJECT

    public:
        NewCharacterController(const QString &clientIdentifier,
                               const QString &clientSecret,
                               QObject *parent = nullptr);
        NewCharacterController(const NewCharacterController &) = default;
        NewCharacterController(NewCharacterController &&) = default;
        virtual ~NewCharacterController() = default;

        virtual QString callback() const override;
        virtual void networkReplyFinished(QNetworkReply *reply) override;

        NewCharacterController &operator =(const NewCharacterController &) = default;
        NewCharacterController &operator =(NewCharacterController &&) = default;

    signals:
        void error(const QString &message);
        void authorizedCharacter(Character::IdType id, const QString &accessToken, const QString &refreshToken);

    private slots:
        void processCharacter(Character::IdType id);
        void handleError(const QString &message);
        void requestSSOAuth(const QUrl &url);

    private:
        ESIOAuth2UnknownCharacterAuthorizationCodeFlow mFlow;
    };
}
