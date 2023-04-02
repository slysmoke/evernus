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

#include <vector>

#include <QAbstractTableModel>

#include "MarketOrderRepository.h"

namespace Evernus
{
    class EveDataProvider;

    class AggregatedStatisticsModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        AggregatedStatisticsModel(const MarketOrderRepository &orderRepo,
                                  const EveDataProvider &dataProvider,
                                  QObject *parent = nullptr);
        virtual ~AggregatedStatisticsModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void clear();

        void reset(Character::IdType characterId,
                   MarketOrderRepository::AggregateColumn groupingColumn,
                   MarketOrderRepository::AggregateOrderColumn orderColumn,
                   int limit,
                   bool includeActive,
                   bool includeNotFulfilled);

    private:
        static const auto idColumn = 0;
        static const auto countColumn = 1;
        static const auto priceColumn = 2;
        static const auto volumeColumn = 3;

        const MarketOrderRepository &mOrderRepo;
        const EveDataProvider &mDataProvider;

        MarketOrderRepository::AggregateColumn mColumn = MarketOrderRepository::AggregateColumn::TypeId;

        MarketOrderRepository::CustomAggregatedData mData;
    };
}
