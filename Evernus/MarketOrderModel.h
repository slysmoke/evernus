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

#include <QAbstractItemModel>
#include <QDateTime>

#include "WalletTransactionsModel.h"
#include "Character.h"

namespace Evernus
{
    class MarketOrder;

    class MarketOrderModel
        : public QAbstractItemModel
    {
    public:
        struct Range
        {
            QDateTime mFrom, mTo;
        };

        struct OrderInfo
        {
            double mOrderPrice = 0.;
            double mMarketPrice = 0.;
            double mTargetPrice = 0.;
            QDateTime mOrderLocalTimestamp;
            QDateTime mMarketLocalTimestamp;
        };

        enum class Grouping
        {
            None,
            Type,
            Group,
            Station
        };

        enum class PriceStatus
        {
            Ok,
            NoData,
            DataTooOld,
        };

        enum class Type
        {
            Neither,
            Buy,
            Sell,
        };

        using QAbstractItemModel::QAbstractItemModel;
        virtual ~MarketOrderModel() = default;

        virtual size_t getOrderCount() const = 0;
        virtual quint64 getVolumeRemaining() const = 0;
        virtual quint64 getVolumeEntered() const = 0;
        virtual double getTotalISK() const = 0;
        virtual double getTotalSize() const = 0;
        virtual Range getOrderRange(const QModelIndex &index) const = 0;
        virtual OrderInfo getOrderInfo(const QModelIndex &index) const = 0;
        virtual EveType::IdType getOrderTypeId(const QModelIndex &index) const = 0;
        virtual Character::IdType getOrderOwnerId(const QModelIndex &index) const = 0;
        virtual const MarketOrder *getOrder(const QModelIndex &index) const = 0;
        virtual WalletTransactionsModel::EntryType getOrderTypeFilter(const QModelIndex &index) const = 0;

        virtual bool shouldShowPriceInfo(const QModelIndex &index) const = 0;

        virtual int getVolumeColumn() const = 0;

        virtual Type getType() const = 0;

        virtual void removeIndexes(const QModelIndexList &indexes) = 0;
    };
}
