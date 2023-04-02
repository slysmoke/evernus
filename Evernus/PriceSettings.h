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

namespace Evernus
{
    namespace PriceSettings
    {
        enum class CopyMode
        {
            DontCopy,
            CopySell,
            CopyBuy
        };

        enum class DataSource
        {
            Orders,
            ItemCost,
            Station
        };

        const auto minMarginDefault = 10.;
        const auto preferredMarginDefault = 30.;
        const auto priceDeltaDefault = 0.01;
        const auto priceDeltaRandomDefault = 0.;
        const auto priceMaxAgeDefault = 1;
        const auto importLogWaitTimeDefault = 1000;
        const auto copyModeDefault = CopyMode::DontCopy;
        const auto priceAltImportDefault = false;
        const auto autoAddCustomItemCostDefault = false;
        const auto costDataSourceDefault = DataSource::Orders;
        const auto shareCostsDefault = false;
        const auto copyNonOverbidPriceDefault = false;
        const auto fpcDefault = false;
        const auto limitSellCopyToCostDefault = false;
        const auto limitSellCopyToTotalCostDefault = false;
        const auto ignoreOrdersWithMinVolumeDefault = false;
        const auto itemConstCostAddDefault = 0.;
        const auto itemRelativeCostAddDefault = 0.;
        const auto showInEveOnFpcDefault = false;
        const auto rangeThresholdDefault = 0;

        const auto minMarginKey = QStringLiteral("prices/margin/min");
        const auto preferredMarginKey = QStringLiteral("prices/margin/preferred");
        const auto copyModeKey = QStringLiteral("prices/copyMode");
        const auto priceDeltaKey = QStringLiteral("prices/delta");
        const auto priceDeltaRandomKey = QStringLiteral("prices/deltaRandom");
        const auto priceAltImportKey = QStringLiteral("prices/import/alt");
        const auto priceMaxAgeKey = QStringLiteral("prices/maxAge");
        const auto importLogWaitTimeKey = QStringLiteral("prices/import/logWaitTime");
        const auto autoAddCustomItemCostKey = QStringLiteral("prices/orders/autoAddCustomItemCost");
        const auto costDataSourceKey = QStringLiteral("prices/dataSource");
        const auto costSourceStationKey = QStringLiteral("prices/costSourceStation");
        const auto shareCostsKey = QStringLiteral("prices/shareCosts");
        const auto refreshPricesWithOrdersKey = QStringLiteral("prices/orders/autoRefresh");
        const auto copyNonOverbidPriceKey = QStringLiteral("prices/orders/copyNonOverbid");
        const auto fpcKey = QStringLiteral("prices/fpc/enabled");
        const auto fpcForwardShortcutKey = QStringLiteral("prices/fpc/shortcut");
        const auto fpcBackwardShortcutKey = QStringLiteral("prices/fpc/backwardShortcut");
        const auto limitSellCopyToCostKey = QStringLiteral("prices/orders/limitSellCopyToCost");
        const auto limitSellCopyToTotalCostKey = QStringLiteral("prices/orders/limitSellCopyToTotalCost");
        const auto ignoreOrdersWithMinVolumeKey = QStringLiteral("prices/ignoreOrdersWithMinVolume");
        const auto itemConstCostAddKey = QStringLiteral("prices/costs/constCost");
        const auto itemRelativeCostAddKey = QStringLiteral("prices/costs/relativeCost");
        const auto showInEveOnFpcKey = QStringLiteral("prices/fpc/showInEve");
        const auto rangeThresholdKey = QStringLiteral("prices/rangeThreshold");
    }
}
