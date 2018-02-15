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

#include <QOAuth2AuthorizationCodeFlow>

#include "Character.h"

class QNetworkRequest;

namespace Evernus
{
    class CharacterRepository;
    class EveDataProvider;

    class ESIOAuth2AuthorizationCodeFlow
        : public QOAuth2AuthorizationCodeFlow
    {
        Q_OBJECT

    public:
        ESIOAuth2AuthorizationCodeFlow(Character::IdType charId,
                                       const CharacterRepository &characterRepo,
                                       const EveDataProvider &dataProvider,
                                       const QString &clientIdentifier,
                                       const QString &clientSecret,
                                       QObject *parent = nullptr);
        ESIOAuth2AuthorizationCodeFlow(const ESIOAuth2AuthorizationCodeFlow &) = default;
        ESIOAuth2AuthorizationCodeFlow(ESIOAuth2AuthorizationCodeFlow &&) = default;
        virtual ~ESIOAuth2AuthorizationCodeFlow() = default;

        // this method exists because of https://bugreports.qt.io/browse/QTBUG-66097
        // TODO: remove when fixed
        void resetStatus();

        ESIOAuth2AuthorizationCodeFlow &operator =(const ESIOAuth2AuthorizationCodeFlow &) = default;
        ESIOAuth2AuthorizationCodeFlow &operator =(ESIOAuth2AuthorizationCodeFlow &&) = default;

    signals:
        void characterConfirmed();

    private slots:
        void checkCharacter();

    private:
        Character::IdType mCharacterId = Character::invalidId;

        const CharacterRepository &mCharacterRepo;
        const EveDataProvider &mDataProvider;

        QString getCharacterName() const;

        static void modifyOAuthparameters(QAbstractOAuth::Stage stage, QVariantMap *params);
    };
}
