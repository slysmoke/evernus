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
#include "ESIOAuth.h"

#include "ESIOAuth2AuthorizationCodeFlow.h"

namespace Evernus
{
    ESIOAuth2AuthorizationCodeFlow::ESIOAuth2AuthorizationCodeFlow(const QString &clientIdentifier,
                                                                   const QString &clientSecret,
                                                                   QObject *parent)
        : QOAuth2AuthorizationCodeFlow{clientIdentifier,
                                       QStringLiteral("https://login.eveonline.com/v2/oauth/authorize"),
                                       QStringLiteral("https://login.eveonline.com/v2/oauth/token"),
                                       nullptr,
                                       parent}
        , mNetworkAccessManager{clientIdentifier, clientSecret}
    {
        setNetworkAccessManager(&mNetworkAccessManager);
        setUserAgent(ESIOAuth::getUserAgent());
        setClientIdentifierSharedKey(clientSecret);
        setScope(QStringLiteral(
            "esi-skills.read_skills.v1 "
            "esi-wallet.read_character_wallet.v1 "
            "esi-assets.read_assets.v1 "
            "esi-ui.open_window.v1 "
            "esi-ui.write_waypoint.v1 "
            "esi-markets.structure_markets.v1 "
            "esi-markets.read_character_orders.v1 "
            "esi-characters.read_blueprints.v1 "
            "esi-contracts.read_character_contracts.v1 "
            "esi-wallet.read_corporation_wallets.v1 "
            "esi-assets.read_corporation_assets.v1 "
            "esi-corporations.read_blueprints.v1 "
            "esi-contracts.read_corporation_contracts.v1 "
            "esi-markets.read_corporation_orders.v1 "
            "esi-industry.read_character_mining.v1 "
            "esi-industry.read_corporation_mining.v1"
        ));
        setContentType(QAbstractOAuth::ContentType::Json);
        setModifyParametersFunction(&ESIOAuth2AuthorizationCodeFlow::modifyOAuthparameters);
    }

    void ESIOAuth2AuthorizationCodeFlow::resetStatus()
    {
        setRefreshToken({});
        setToken({});
        setStatus(Status::NotAuthenticated);
    }

    void ESIOAuth2AuthorizationCodeFlow::modifyOAuthparameters(QAbstractOAuth::Stage stage, QVariantMap *params)
    {
        if (stage == QAbstractOAuth::Stage::RequestingAccessToken)
        {
            Q_ASSERT(params != nullptr);

            // we already pass those in the header
            params->remove(QStringLiteral("client_id"));
            params->remove(QStringLiteral("client_secret"));
        }
    }
}
