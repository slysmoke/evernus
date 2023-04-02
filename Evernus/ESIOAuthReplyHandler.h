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

#include "Character.h"

namespace Evernus
{
    class ESIOAuthReplyHandler
        : public QAbstractOAuthReplyHandler
    {
        Q_OBJECT

    public:
        ESIOAuthReplyHandler(Character::IdType charId,
                             QString scope,
                             QObject *parent = nullptr);
        ESIOAuthReplyHandler(const ESIOAuthReplyHandler &) = default;
        ESIOAuthReplyHandler(ESIOAuthReplyHandler &&) = default;
        virtual ~ESIOAuthReplyHandler() = default;

        virtual QString callback() const override;
        virtual void networkReplyFinished(QNetworkReply *reply) override;

        ESIOAuthReplyHandler &operator =(const ESIOAuthReplyHandler &) = default;
        ESIOAuthReplyHandler &operator =(ESIOAuthReplyHandler &&) = default;

    signals:
        void error(const QString &message);

    public slots:
        void handleAuthReply(Character::IdType charId, const QVariantMap &data);

    private:
        Character::IdType mCharId = Character::invalidId;
        QString mScope;
    };
}
