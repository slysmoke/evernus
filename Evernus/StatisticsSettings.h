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

#include <QColor>

namespace Evernus
{
    namespace StatisticsSettings
    {
        const auto combineCorpAndCharPlotsDefault = false;
        const QColor statisticsAssetPlotColorDefault = Qt::blue;
        const QColor statisticsCorpAssetPlotColorDefault = Qt::darkGray;
        const QColor statisticsWalletPlotColorDefault = Qt::red;
        const QColor statisticsCorpWalletPlotColorDefault = Qt::cyan;
        const QColor statisticsBuyOrderPlotColorDefault = Qt::yellow;
        const QColor statisticsSellOrderPlotColorDefault = Qt::magenta;
        const QColor statisticsTotalPlotColorDefault = Qt::green;
        const auto combineStatisticsDefault = false;
        const auto automaticSnapshotsDefault = true;

        const auto combineCorpAndCharPlotsKey = "prices/combineCorpAndCharPlots";
        const auto statisticsAssetPlotColorKey = "ui/appearance/statistics/assetColor";
        const auto statisticsCorpAssetPlotColorKey = "ui/appearance/statistics/corpAssetColor";
        const auto statisticsWalletPlotColorKey = "ui/appearance/statistics/walletColor";
        const auto statisticsCorpWalletPlotColorKey = "ui/appearance/statistics/corpWalletColor";
        const auto statisticsBuyOrderPlotColorKey = "ui/appearance/statistics/buyOrderColor";
        const auto statisticsSellOrderPlotColorKey = "ui/appearance/statistics/sellOrderColor";
        const auto statisticsTotalPlotColorKey = "ui/appearance/statistics/totalColor";
        const auto combineStatisticsKey = "ui/combine/statistics";
        const auto automaticSnapshotsKey = "statistics/automaticSnapshots";
    }
}
