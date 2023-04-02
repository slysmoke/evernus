/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for mScrapmetal details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "ScrapmetalReprocessingArbitrageModel.h"
#include "ReprocessingArbitrageWidget.h"

namespace Evernus
{
    class ScrapmetalReprocessingArbitrageWidget
        : public ReprocessingArbitrageWidget
    {
    public:
        ScrapmetalReprocessingArbitrageWidget(const EveDataProvider &dataProvider,
                                              const MarketDataProvider &marketDataProvider,
                                              const RegionStationPresetRepository &regionStationPresetRepository,
                                              QWidget *parent = nullptr);
        ScrapmetalReprocessingArbitrageWidget(const ScrapmetalReprocessingArbitrageWidget &) = default;
        ScrapmetalReprocessingArbitrageWidget(ScrapmetalReprocessingArbitrageWidget &&) = default;
        virtual ~ScrapmetalReprocessingArbitrageWidget() = default;

        ScrapmetalReprocessingArbitrageWidget &operator =(const ScrapmetalReprocessingArbitrageWidget &) = default;
        ScrapmetalReprocessingArbitrageWidget &operator =(ScrapmetalReprocessingArbitrageWidget &&) = default;

    private:
        ScrapmetalReprocessingArbitrageModel mDataModel;
        QSortFilterProxyModel mDataProxy;
    };
}
