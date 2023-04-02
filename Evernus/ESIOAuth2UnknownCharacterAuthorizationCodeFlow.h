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

#include "ESIOAuth2AuthorizationCodeFlow.h"
#include "Character.h"

class QNetworkRequest;

namespace Evernus
{
    class ESIOAuth2UnknownCharacterAuthorizationCodeFlow
        : public ESIOAuth2AuthorizationCodeFlow
    {
        Q_OBJECT

    public:
        ESIOAuth2UnknownCharacterAuthorizationCodeFlow(const QString &clientIdentifier,
                                                       const QString &clientSecret,
                                                       QObject *parent = nullptr);
        ESIOAuth2UnknownCharacterAuthorizationCodeFlow(const ESIOAuth2UnknownCharacterAuthorizationCodeFlow &) = default;
        ESIOAuth2UnknownCharacterAuthorizationCodeFlow(ESIOAuth2UnknownCharacterAuthorizationCodeFlow &&) = default;
        virtual ~ESIOAuth2UnknownCharacterAuthorizationCodeFlow() = default;

        ESIOAuth2UnknownCharacterAuthorizationCodeFlow &operator =(const ESIOAuth2UnknownCharacterAuthorizationCodeFlow &) = default;
        ESIOAuth2UnknownCharacterAuthorizationCodeFlow &operator =(ESIOAuth2UnknownCharacterAuthorizationCodeFlow &&) = default;

    signals:
        void characterConfirmed(Character::IdType id);

    private slots:
        void checkCharacter();
    };
}
