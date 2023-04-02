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

#include <unordered_map>

#include "MarketOrderRepository.h"
#include "MarketOrderProvider.h"

namespace Evernus
{
    class CachingMarketOrderProvider
        : public QObject
        , public MarketOrderProvider
    {
        Q_OBJECT

    public:
        explicit CachingMarketOrderProvider(const MarketOrderRepository &orderRepo);
        virtual ~CachingMarketOrderProvider() = default;

        virtual OrderList getSellOrders(Character::IdType characterId) const override;
        virtual OrderList getBuyOrders(Character::IdType characterId) const override;
        virtual OrderList getArchivedOrders(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const override;

        virtual OrderList getSellOrdersForCorporation(quint64 corporationId) const override;
        virtual OrderList getBuyOrdersForCorporation(quint64 corporationId) const override;
        virtual OrderList getArchivedOrdersForCorporation(quint64 corporationId, const QDateTime &from, const QDateTime &to) const override;

        virtual void removeOrder(MarketOrder::IdType id) override;

        virtual void setOrderNotes(MarketOrder::IdType id, const QString &notes) override;
        virtual void setOrderStation(MarketOrder::IdType orderId, uint stationId) override;
        virtual void setOrderColorTag(MarketOrder::IdType orderId, const QColor &color) override;

        void clearOrdersForCharacter(Character::IdType id) const;
        void clearOrdersForCorporation(uint id) const;
        void clearArchived() const;

    signals:
        void orderChanged();

    private:
        typedef std::unordered_map<Character::IdType, OrderList> MarketOrderMap;
        typedef std::unordered_map<quint64, OrderList> CorpMarketOrderMap;

        const MarketOrderRepository &mOrderRepo;

        mutable MarketOrderMap mSellOrders;
        mutable MarketOrderMap mBuyOrders;
        mutable MarketOrderMap mArchivedOrders;

        mutable CorpMarketOrderMap mCorpSellOrders;
        mutable CorpMarketOrderMap mCorpBuyOrders;
        mutable CorpMarketOrderMap mCorpArchivedOrders;

        void clearAll();
    };
}
