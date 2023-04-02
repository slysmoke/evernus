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

#include "VolumeType.h"
#include "PriceType.h"

namespace Evernus
{
    namespace MarketAnalysisSettings
    {
        const auto dontSaveLargeOrdersDefault = true;
        const auto ignoreExistingOrdersDefault = true;
        const auto smaDaysDefault = 20;
        const auto macdFastDaysDefault = 5;
        const auto macdSlowDaysDefault = 15;
        const auto macdEmaDaysDefault = 5;
        const auto showLegendDefault = true;
        const auto discardBogusOrdersDefault = true;
        const auto bogusOrderThresholdDefault = 0.9;
        const auto useSkillsForDifferenceDefault = false;
        const auto importingAggrDaysDefault = 7;
        const auto importingAnalysisDaysDefault = 180;
        const auto importingCollateralPriceTypeDefault = static_cast<int>(PriceType::Buy);
        const auto importingHideEmptySellOrdersDefault = true;
        const auto reprocessingStationEfficiencyDefault = 50.;
        const auto reprocessingIncludeStationTaxDefault = true;
        const auto reprocessingIgnoreMinVolumeDefault = true;
        const auto reprocessingSellVolumeLimitDefault = 10;
        const auto reprocessingOnlyHighSecDefault = true;
        const auto reprocessingCustomStationTaxDefault = false;
        const auto reprocessingCustomStationTaxValueDefault = 0.;
        const auto volumeGraphTypeDefault = static_cast<int>(VolumeType::Volume);
        const auto typeAggregatedChartDurationDefault = 90;
        const auto ignorePricePercetilesDefault = false;
        const auto avgDaysDefault = 30;

        const auto dontSaveLargeOrdersKey = QStringLiteral("marketAnalysis/dontSaveOrders");
        const auto minVolumeFilterKey = QStringLiteral("marketAnalysis/filter/minVolume");
        const auto maxVolumeFilterKey = QStringLiteral("marketAnalysis/filter/maxVolume");
        const auto minMarginFilterKey = QStringLiteral("marketAnalysis/filter/minMargin");
        const auto maxMarginFilterKey = QStringLiteral("marketAnalysis/filter/maxMargin");
        const auto minBuyPriceFilterKey = QStringLiteral("marketAnalysis/filter/minBuyPrice");
        const auto maxBuyPriceFilterKey = QStringLiteral("marketAnalysis/filter/maxBuyPrice");
        const auto minSellPriceFilterKey = QStringLiteral("marketAnalysis/filter/minSellPrice");
        const auto maxSellPriceFilterKey = QStringLiteral("marketAnalysis/filter/maxSellPrice");
        const auto minAvgVolumeFilterKey = QStringLiteral("marketAnalysis/filter/minAvgVolume");
        const auto maxAvgVolumeFilterKey = QStringLiteral("marketAnalysis/filter/maxAvgVolume");
        const auto minPriceDifferenceFilterKey = QStringLiteral("marketAnalysis/filter/minPriceDifference");
        const auto maxPriceDifferenceFilterKey = QStringLiteral("marketAnalysis/filter/maxPriceDifference");
        const auto ignorePricePercetilesKey = QStringLiteral("marketAnalysis/region/ignorePricePercetiles");
        const auto avgDaysKey = QStringLiteral("marketAnalysis/region/avgDays");
        const auto smaDaysKey = QStringLiteral("marketAnalysis/smaDays");
        const auto lastRegionKey = QStringLiteral("marketAnalysis/lastRegion");
        const auto macdFastDaysKey = QStringLiteral("marketAnalysis/macd/fastDays");
        const auto macdSlowDaysKey = QStringLiteral("marketAnalysis/macd/slowDays");
        const auto macdEmaDaysKey = QStringLiteral("marketAnalysis/macd/emaDays");
        const auto showLegendKey = QStringLiteral("marketAnalysis/showLegend");
        const auto ignoreExistingOrdersKey = QStringLiteral("marketAnalysis/ignoreExistingOrders");
        const auto discardBogusOrdersKey = QStringLiteral("marketAnalysis/discardBogusOrders");
        const auto bogusOrderThresholdKey = QStringLiteral("marketAnalysis/bogusOrderThreshold");
        const auto srcRegionKey = QStringLiteral("marketAnalysis/interRegion/srcRegion");
        const auto dstRegionKey = QStringLiteral("marketAnalysis/interRegion/dstRegion");
        const auto srcStationKey = QStringLiteral("marketAnalysis/interRegion/srcStation");
        const auto dstStationKey = QStringLiteral("marketAnalysis/interRegion/dstStation");
        const auto useSkillsForDifferenceKey = QStringLiteral("marketAnalysis/useSkillsForDifference");
        const auto srcImportStationKey = QStringLiteral("marketAnalysis/importing/srcStation");
        const auto dstImportStationKey = QStringLiteral("marketAnalysis/importing/dstStation");
        const auto importingAggrDaysKey = QStringLiteral("marketAnalysis/importing/aggrDays");
        const auto importingAnalysisDaysKey = QStringLiteral("marketAnalysis/importing/analysisDays");
        const auto importingPricePerM3Key = QStringLiteral("marketAnalysis/importing/pricePerM3");
        const auto importingCollateralKey = QStringLiteral("marketAnalysis/importing/collateral");
        const auto importingCollateralPriceTypeKey = QStringLiteral("marketAnalysis/importing/collateralPriceType");
        const auto importingHideEmptySellOrdersKey = QStringLiteral("marketAnalysis/importing/hideEmptySellOrders");
        const auto reprocessingStationEfficiencyKey = QStringLiteral("marketAnalysis/reprocessing/stationEfficiency");
        const auto reprocessingIncludeStationTaxKey = QStringLiteral("marketAnalysis/reprocessing/includeStationTax");
        const auto reprocessingIgnoreMinVolumeKey = QStringLiteral("marketAnalysis/reprocessing/ignoreMinVolume");
        const auto reprocessingSrcStationKey = QStringLiteral("marketAnalysis/reprocessing/srcStation");
        const auto reprocessingDstStationKey = QStringLiteral("marketAnalysis/reprocessing/dstStation");
        const auto reprocessingSrcRegionKey = QStringLiteral("marketAnalysis/reprocessing/srcRegion");
        const auto reprocessingDstRegionKey = QStringLiteral("marketAnalysis/reprocessing/dstRegion");
        const auto reprocessingSellVolumeLimitKey = QStringLiteral("marketAnalysis/reprocessing/sellVolumeLimit");
        const auto reprocessingOnlyHighSecKey = QStringLiteral("marketAnalysis/reprocessing/onlyHighSec");
        const auto reprocessingCustomStationTaxKey = QStringLiteral("marketAnalysis/reprocessing/customStationTax");
        const auto reprocessingCustomStationTaxValueKey = QStringLiteral("marketAnalysis/reprocessing/customStationTaxValue");
        const auto typeAggregatedChartDurationKey = QStringLiteral("marketAnalysis/typeAggregatedChart/duration");
        const auto volumeGraphTypeKey = QStringLiteral("marketAnalysis/typeAggregatedChart/volumeType");
    }
}
