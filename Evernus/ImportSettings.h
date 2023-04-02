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

#include <QString>

namespace Evernus::ImportSettings
{
    enum class SmtpConnectionSecurity
    {
        None,
        STARTTLS,
        TLS
    };

    enum class PriceImportSource
    {
        Web,
        Logs,
    };

    enum class MarketOrderImportSource
    {
        API,
        Logs
    };

    enum class MarketOrderImportType
    {
        Auto,
        Individual,
        Whole
    };

    const auto importSkillsDefault = true;
    const auto importPortraitDefault = true;
    const auto importAssetsDefault = true;
    const auto autoUpdateAssetValueDefault = true;
    const auto updateOnlyFullAssetValueDefault = false;
    const auto importTimerDefault = 60;
    const auto autoImportEnabledDefault = false;
    const auto autoImportTimerDefault = 60;
    const auto emailNotificationsEnabledDefault = true;
    const auto smtpHostDefault = QStringLiteral("localhost");
    const auto smtpPortDefault = 25;
    const auto priceImportSourceDefault = PriceImportSource::Web;
    const auto corpOrdersWithCharacterDefault = true;
    const auto importContractsDefault = true;
    const auto ignoreCachedImportDefault = true;
    const auto marketOrderImportSourceDefault = MarketOrderImportSource::API;
    const auto makeCorpSnapshotsDefault = true;
    const auto useCustomAssetStationDefault = false;
    const auto importAllCharactersDefault = true;
    const auto corpWalletDivisionDefault = 1;
    const auto marketOrderImportTypeDefault = MarketOrderImportType::Auto;
    const auto csvSeparatorDefault = QChar(',');
    const auto maxCitadelAccessAgeDefault = 7;
    const auto importMiningLedgerDefault = false;
    const auto citadelAccessCacheWarningDefault = true;
    const auto clearExistingCitadelsDefault = false;

    const auto smtpCryptKey = Q_UINT64_C(0x740376004af2acc9);

    const auto importSkillsKey = QStringLiteral("import/character/importSkills");
    const auto importPortraitKey = QStringLiteral("import/character/importPortrait");
    const auto importAssetsKey = QStringLiteral("import/assets/import");
    const auto autoUpdateAssetValueKey = QStringLiteral("import/assets/autoUpdateValue");
    const auto updateOnlyFullAssetValueKey = QStringLiteral("import/assets/updateOnlyFullAssetValue");
    const auto maxCharacterAgeKey = QStringLiteral("import/character/maxAge");
    const auto maxAssetListAgeKey = QStringLiteral("import/assetList/maxAge");
    const auto maxWalletAgeKey = QStringLiteral("import/wallet/maxAge");
    const auto maxMarketOrdersAgeKey = QStringLiteral("import/marketOrders/maxAge");
    const auto maxContractsAgeKey = QStringLiteral("import/contracts/maxAge");
    const auto updateCorpDataKey = QStringLiteral("import/corp/update");
    const auto makeCorpSnapshotsKey = QStringLiteral("import/corp/makeSnapshots");
    const auto autoImportEnabledKey = QStringLiteral("import/autoImport/enabled");
    const auto autoImportTimeKey = QStringLiteral("import/autoImport/time");
    const auto emailNotificationsEnabledKey = QStringLiteral("import/email/enabled");
    const auto emailNotificationAddressKey = QStringLiteral("import/email/address");
    const auto smtpConnectionSecurityKey = QStringLiteral("import/email/connectionSecurity");
    const auto smtpHostKey = QStringLiteral("import/email/smtpHost");
    const auto smtpPortKey = QStringLiteral("import/email/smtpPort");
    const auto smtpUserKey = QStringLiteral("import/email/smtpUser");
    const auto smtpPasswordKey = QStringLiteral("import/email/smtpPassword");
    const auto priceImportSourceKey = QStringLiteral("import/source/price");
    const auto corpOrdersWithCharacterKey = QStringLiteral("import/corp/showWithCharacter");
    const auto importContractsKey = QStringLiteral("import/contracts/import");
    const auto ignoreCachedImportKey = QStringLiteral("import/ignoreCached");
    const auto marketOrderImportSourceKey = QStringLiteral("import/source/marketOrder");
    const auto useCustomAssetStationKey = QStringLiteral("import/assets/useCustomStation");
    const auto customAssetStationKey = QStringLiteral("import/assets/customStation");
    const auto importAllCharactersKey = QStringLiteral("import/allCharacters");
    const auto corpWalletDivisionKey = QStringLiteral("import/corp/walletDivision");
    const auto marketOrderImportTypeKey = QStringLiteral("import/marketOrderType");
    const auto itemPricesFileDirKey = QStringLiteral("import/costs/fileDir");
    const auto csvSeparatorKey = QStringLiteral("import/csvSeparator");
    const auto maxCitadelAccessAgeKey = QStringLiteral("import/citadelAccess/maxAge");
    const auto importMiningLedgerKey = QStringLiteral("import/character/importMiningLedger");
    const auto maxMiningLedgerAgeKey = QStringLiteral("import/miningLedger/maxAge");
    const auto citadelAccessCacheWarningKey = QStringLiteral("import/citadelAccessCacheWarning");
    const auto clearExistingCitadelsKey = QStringLiteral("import/clearExistingCitadels");
}
