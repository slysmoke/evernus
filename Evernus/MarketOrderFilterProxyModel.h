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

#include <QJSEngine>

#include "LeafFilterProxyModel.h"

namespace Evernus
{
    class ItemCostProvider;
    class EveDataProvider;
    class MarketOrder;

    class MarketOrderFilterProxyModel
        : public LeafFilterProxyModel
    {
        Q_OBJECT

    public:
        enum StatusFilter
        {
            Changed             = 0x01,
            Active              = 0x02,
            Fulfilled           = 0x04,
            Cancelled           = 0x08,
            Pending             = 0x10,
            CharacterDeleted    = 0x20,
            Expired             = 0x40,

            EveryStatus         = Changed | Active | Fulfilled | Cancelled | Pending | CharacterDeleted | Expired
        };
        Q_DECLARE_FLAGS(StatusFilters, StatusFilter)

        enum PriceStatusFilter
        {
            Ok                  = 0x1,
            NoData              = 0x2,
            DataTooOld          = 0x4,

            EveryPriceStatus    = Ok | NoData | DataTooOld
        };
        Q_DECLARE_FLAGS(PriceStatusFilters, PriceStatusFilter)

        static const StatusFilters defaultStatusFilter;
        static const PriceStatusFilters defaultPriceStatusFilter;

        MarketOrderFilterProxyModel(const EveDataProvider &dataProvider,
                                    const ItemCostProvider &itemCostProvider,
                                    QObject *parent = nullptr);
        virtual ~MarketOrderFilterProxyModel() = default;

        virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

        StatusFilters getStatusFilter() const;
        PriceStatusFilters getPriceStatusFilter() const;

    signals:
        void scriptError(const QString &message);

    public slots:
        void setStatusFilter(const StatusFilters &filter);
        void setPriceStatusFilter(const PriceStatusFilters &filter);

        void setTextFilter(const QString &text, bool script);

        void unscheduleScriptError();

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    private:
        const EveDataProvider &mDataProvider;
        const ItemCostProvider &mItemCostProvider;

        StatusFilters mStatusFilter = defaultStatusFilter;
        PriceStatusFilters mPriceStatusFilter = defaultPriceStatusFilter;

        mutable QJSEngine mEngine;
        mutable QJSValue mFilterFunction;

        mutable bool mScriptErrorScheduled = false;

        mutable QStringList mExceptions;

        bool acceptsStatus(const MarketOrder &order) const;
        bool acceptsPriceStatus(const MarketOrder &order) const;
        bool acceptsByScript(const MarketOrder &order) const;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Evernus::MarketOrderFilterProxyModel::StatusFilters)
Q_DECLARE_OPERATORS_FOR_FLAGS(Evernus::MarketOrderFilterProxyModel::PriceStatusFilters)
