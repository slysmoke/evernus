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

#include "ReprocessingArbitrageModel.h"

namespace Evernus
{
    class ScrapmetalReprocessingArbitrageModel
        : public ReprocessingArbitrageModel
    {
        Q_OBJECT

    public:
        explicit ScrapmetalReprocessingArbitrageModel(const EveDataProvider &dataProvider,
                                                      QObject *parent = nullptr);
        ScrapmetalReprocessingArbitrageModel(const ScrapmetalReprocessingArbitrageModel &) = default;
        ScrapmetalReprocessingArbitrageModel(ScrapmetalReprocessingArbitrageModel &&) = default;
        virtual ~ScrapmetalReprocessingArbitrageModel() = default;

        virtual void setOrderData(const std::vector<ExternalOrder> &orders,
                                  PriceType dstPriceType,
                                  const RegionList &srcRegions,
                                  const RegionList &dstRegions,
                                  quint64 srcStation,
                                  quint64 dstStation,
                                  bool useStationTax,
                                  bool ignScrapmetalMinVolume,
                                  bool onlyHighSec,
                                  double baseYield,
                                  double sellVolumeLimit,
                                  const std::optional<double> &customStationTax) override;

        ScrapmetalReprocessingArbitrageModel &operator =(const ScrapmetalReprocessingArbitrageModel &) = default;
        ScrapmetalReprocessingArbitrageModel &operator =(ScrapmetalReprocessingArbitrageModel &&) = default;

    private:
        std::unordered_set<uint> mOreGroups;

        void insertOreGroup(const QString &groupName);
    };
}
