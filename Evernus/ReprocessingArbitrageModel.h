/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for m details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <unordered_set>
#include <memory>
#include <vector>

#include <optional>

#include <QAbstractTableModel>

#include "ModelWithTypes.h"
#include "Character.h"
#include "PriceType.h"
#include "EveType.h"

namespace Evernus
{
    class EveDataProvider;
    class ExternalOrder;

    class ReprocessingArbitrageModel
        : public QAbstractTableModel
        , public ModelWithTypes
    {
        Q_OBJECT

    public:
        using RegionList = std::unordered_set<uint>;

        using QAbstractTableModel::QAbstractTableModel;

        explicit ReprocessingArbitrageModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        ReprocessingArbitrageModel(const ReprocessingArbitrageModel &) = default;
        ReprocessingArbitrageModel(ReprocessingArbitrageModel &&) = default;
        virtual ~ReprocessingArbitrageModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        virtual EveType::IdType getTypeId(const QModelIndex &index) const override;

        void setCharacter(std::shared_ptr<Character> character);

        void reset();

        virtual void setOrderData(const std::vector<ExternalOrder> &orders,
                                  PriceType dstPriceType,
                                  const RegionList &srcRegions,
                                  const RegionList &dstRegions,
                                  quint64 srcStation,
                                  quint64 dstStation,
                                  bool useStationTax,
                                  bool ignoreMinVolume,
                                  bool onlyHighSec,
                                  double baseYield,
                                  double sellVolumeLimit,
                                  const std::optional<double> &customStationTax) = 0;

        ReprocessingArbitrageModel &operator =(const ReprocessingArbitrageModel &) = default;
        ReprocessingArbitrageModel &operator =(ReprocessingArbitrageModel &&) = default;

    protected:
        struct ItemData
        {
            EveType::IdType mId = EveType::invalidId;
            double mTotalProfit = 0.;
            double mTotalCost = 0.;
            double mMargin = 0.;
            quint64 mVolume = 0;
        };

        const EveDataProvider &mDataProvider;

        std::vector<ItemData> mData;

        std::shared_ptr<Character> mCharacter;

        static auto getValidRegionFilter()
        {
            return [](const auto &regions, const auto &order) {
                return regions.find(order.getRegionId()) != std::end(regions);
            };
        }

        static auto getValidStationFilter()
        {
            return [](auto stationId, const auto &order) {
                return stationId == 0 || order.getStationId() == stationId;
            };
        }

    private:
        enum
        {
            nameColumn,
            volumeColumn,
            totalProfitColumn,
            totalCostColumn,
            differenceColumn,
            marginColumn,

            numColumns
        };
    };
}
