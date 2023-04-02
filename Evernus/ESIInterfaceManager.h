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

#include <QDateTime>
#include <QObject>
#include <QString>

#include "QObjectDeleteLaterDeleter.h"
#include "ESIInterfaceErrorLimiter.h"
#include "CitadelAccessCache.h"
#include "ESIInterface.h"
#include "Character.h"
#include "ESIOAuth.h"

class QByteArray;
class QUrl;

namespace Evernus
{
    class CharacterRepository;
    class EveDataProvider;
    class ESIInterface;

    class ESIInterfaceManager final
        : public QObject
    {
        Q_OBJECT

    public:
        ESIInterfaceManager(QString clientId,
                            QString clientSecret,
                            const CharacterRepository &characterRepo,
                            const EveDataProvider &dataProvider,
                            QObject *parent = nullptr);
        ESIInterfaceManager(const ESIInterfaceManager &) = delete;
        ESIInterfaceManager(ESIInterfaceManager &&) = default;
        virtual ~ESIInterfaceManager();

        void clearRefreshTokens();

        void processSSOAuthorizationCode(Character::IdType charId, const QByteArray &code);
        void cancelSsoAuth(Character::IdType charId);
        void setTokens(Character::IdType id, const QString &accessToken, const QString &refreshToken);

        const ESIInterface &getInterface() const;

        const CitadelAccessCache &getCitadelAccessCache() const noexcept;
        CitadelAccessCache &getCitadelAccessCache() noexcept;

        QString getClientId() const;
        QString getClientSecret() const;

        ESIInterfaceManager &operator =(const ESIInterfaceManager &) = delete;
        ESIInterfaceManager &operator =(ESIInterfaceManager &&) = default;

    signals:
        void ssoAuthRequested(Character::IdType charId, const QUrl &url);

    private:
        QString mClientId;
        QString mClientSecret;

        CitadelAccessCache mCitadelAccessCache;
        ESIInterfaceErrorLimiter mErrorLimiter;
        ESIOAuth mOAuth;

        ESIInterface mInterface;

        void readCitadelAccessCache();
        void writeCitadelAccessCache();

        static QString getCachePath();
    };
}
