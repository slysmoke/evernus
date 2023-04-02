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
#include <QLocale>

#include "EveDataProvider.h"
#include "TextUtils.h"

#include "AggregatedStatisticsModel.h"

namespace Evernus
{
    AggregatedStatisticsModel::AggregatedStatisticsModel(const MarketOrderRepository &orderRepo,
                                                         const EveDataProvider &dataProvider,
                                                         QObject *parent)
        : QAbstractTableModel{parent}
        , mOrderRepo{orderRepo}
        , mDataProvider{dataProvider}
    {
    }

    int AggregatedStatisticsModel::columnCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (4);
    }

    QVariant AggregatedStatisticsModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return QVariant{};

        const auto &record = mData[index.row()];
        const auto column = index.column();

        if (role == Qt::DisplayRole)
        {
            QLocale locale;

            switch (column) {
            case idColumn:
                if (mColumn == MarketOrderRepository::AggregateColumn::TypeId)
                    return mDataProvider.getTypeName(record.first);

                return mDataProvider.getLocationName(record.first);
            case countColumn:
                return locale.toString(record.second.mCount);
            case priceColumn:
                return TextUtils::currencyToString(record.second.mPriceSum, locale);
            case volumeColumn:
                return locale.toString(record.second.mVolume);
            }
        }
        else if (role == Qt::UserRole)
        {
            switch (column) {
            case idColumn:
                return record.first;
            case countColumn:
                return record.second.mCount;
            case priceColumn:
                return record.second.mPriceSum;
            case volumeColumn:
                return record.second.mVolume;
            }
        }

        return QVariant{};
    }

    QVariant AggregatedStatisticsModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section) {
            case idColumn:
                return tr("Id");
            case countColumn:
                return tr("Count");
            case priceColumn:
                return tr("Price sum");
            case volumeColumn:
                return tr("Volume");
            }
        }

        return QVariant{};
    }

    int AggregatedStatisticsModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void AggregatedStatisticsModel::clear()
    {
        beginResetModel();
        mData.clear();
        endResetModel();
    }

    void AggregatedStatisticsModel::reset(Character::IdType characterId,
                                          MarketOrderRepository::AggregateColumn groupingColumn,
                                          MarketOrderRepository::AggregateOrderColumn orderColumn,
                                          int limit,
                                          bool includeActive,
                                          bool includeNotFulfilled)
    {
        beginResetModel();
        mColumn = groupingColumn;
        mData = mOrderRepo.getCustomAggregatedData(characterId, mColumn, orderColumn, limit, includeActive, includeNotFulfilled);
        endResetModel();
    }
}
