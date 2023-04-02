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

#include <unordered_set>

#include "ExternalOrderModel.h"
#include "ExternalOrder.h"
#include "MarketOrder.h"

namespace Evernus
{
    class ExternalOrderRepository;
    class CharacterRepository;
    class MarketOrderProvider;
    class ItemCostProvider;

    class ExternalOrderBuyModel
        : public ExternalOrderModel
    {
        Q_OBJECT

    public:
        ExternalOrderBuyModel(const EveDataProvider &dataProvider,
                              const ExternalOrderRepository &orderRepo,
                              const CharacterRepository &characterRepo,
                              const MarketOrderProvider &orderProvider,
                              const MarketOrderProvider &corpOrderProvider,
                              const ItemCostProvider &costProvider,
                              QObject *parent = nullptr);
        virtual ~ExternalOrderBuyModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        virtual int getPriceColumn() const override;
        virtual Qt::SortOrder getPriceSortOrder() const override;
        virtual int getVolumeColumn() const override;

        virtual void setCharacter(Character::IdType id) override;

        virtual void reset() override;

        double getPrice(const QModelIndex &index) const;

    private:
        enum
        {
            stationColumn,
            deviationColumn,
            priceColumn,
            totalCostColumn,
            volumeColumn,
            rangeColumn,
            minQunatityColumn,
            totalSizeColumn,
            issuedColumn,
            durationColumn,
            updatedColumn,
            regionColumn,

            numUngroupedColumns
        };

        enum
        {
            lowestPriceColumn = groupByColumn + 1,
            medianPriceColumn,
            highestPriceColumn,
            groupedTotalCostColumn = 5,
            ordersColumn,
            groupedTotalSizeColumn,

            numGroupedColumns
        };

        const ExternalOrderRepository &mOrderRepo;
        const CharacterRepository &mCharacterRepo;
        const MarketOrderProvider &mOrderProvider;
        const MarketOrderProvider &mCorpOrderProvider;
        const ItemCostProvider &mCostProvider;

        Character::IdType mCharacterId = Character::invalidId;

        std::unordered_set<MarketOrder::IdType> mOwnOrders;

        double computeDeviation(const ExternalOrder &order) const;

        virtual void refreshGroupedData() override;

        virtual QVariant getUngroupedData(int column, int role, const ExternalOrder &order) const override;
        virtual QVariant getGenericGroupedData(int column, int role, const GroupedData &data) const override;

        template<class Id, Id (ExternalOrder::* Func)() const>
        void fillGroupedData();
    };
}
