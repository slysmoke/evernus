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
    namespace UISettings
    {
        const auto autoCloseTasksDefault = true;
        const auto autoCopyPriceFromInfoDefault = true;
        const auto minimizeToTrayDefault = false;
        const auto minimizeByMarginToolDefault = true;
        const auto translationPath = QStringLiteral("/translations");
        const auto plotNumberFormatDefault = QStringLiteral("f");
        const auto usePackagedVolumeDefault = false;
        const auto omitCurrencySymbolDefault = false;
        const auto mainWindowAlwaysOnTopDefault = false;
        const auto useUTCDatesDefault = false;
        const auto combineAssetsDefault = false;
        const auto combineJournalDefault = false;
        const auto combineTransactionsDefault = false;
        const auto columnDelimiterDefault = '\t';
        const auto applyDateFormatToGraphsDefault = false;
        const auto showMarginToolSampleDataDefault = true;
        const auto combineReportsDefault = false;
        const auto combineReportsWithCorpDefault = false;

        const auto autoCloseTasksKey = QStringLiteral("ui/tasks/autoClose");
        const auto autoCopyPriceFromInfoKey = QStringLiteral("ui/price/copyFromInfo");
        const auto marketOrderStateFilterKey = QStringLiteral("ui/orders/stateFilter");
        const auto marketOrderPriceStatusFilterKey = QStringLiteral("ui/orders/priceStatusFilter");
        const auto minimizeToTrayKey = QStringLiteral("ui/mainWindow/minimizeToTray");
        const auto minimizeByMarginToolKey = QStringLiteral("ui/mainWindow/minimizeByMarginTool");
        const auto languageKey = QStringLiteral("ui/global/language");
        const auto dateTimeFormatKey = QStringLiteral("ui/global/dateTimeFormat");
        const auto plotNumberFormatKey = QStringLiteral("ui/plot/numberFormat");
        const auto contractStatusFilterKey = QStringLiteral("ui/contracts/statusFilter");
        const auto headerStateKey = QStringLiteral("ui/header/state/%1");
        const auto usePackagedVolumeKey = QStringLiteral("ui/global/usePackagedVolume");
        const auto tabShowStateKey = QStringLiteral("ui/mainWindow/tabShowState/%1");
        const auto tabShowStateParentKey = QStringLiteral("ui/mainWindow/tabShowState");
        const auto omitCurrencySymbolKey = QStringLiteral("ui/global/omitCurrencySymbol");
        const auto mainWindowAlwaysOnTopKey = QStringLiteral("ui/mainWindow/alwaysOnTop");
        const auto useUTCDatesKey = QStringLiteral("ui/global/useUTCDates");
        const auto lastCharacterKey = QStringLiteral("ui/global/lastCharacter");
        const auto combineAssetsKey = QStringLiteral("ui/combine/assets");
        const auto combineJournalKey = QStringLiteral("ui/combine/journal");
        const auto combineTransactionsKey = QStringLiteral("ui/combine/transactions");
        const auto columnDelimiterKey = QStringLiteral("ui/global/columnDelimiter");
        const auto applyDateFormatToGraphsKey = QStringLiteral("ui/global/applyDateTimeFormatToGraphs");
        const auto showMarginToolSampleDataKey = QStringLiteral("ui/marginTool/showSamples");
        const auto combineReportsKey = QStringLiteral("ui/combine/reports");
        const auto combineReportsWithCorpKey = QStringLiteral("ui/combine/reportsWithCorp");
    }
}
