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
#include "CachingMarketOrderProvider.h"

namespace Evernus
{
    CachingMarketOrderProvider::CachingMarketOrderProvider(const MarketOrderRepository &orderRepo)
        : QObject{}
        , MarketOrderProvider{}
        , mOrderRepo{orderRepo}
    {
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider::getSellOrders(Character::IdType characterId) const
    {
        auto it = mSellOrders.find(characterId);
        if (it == std::end(mSellOrders))
            it = mSellOrders.emplace(characterId, mOrderRepo.fetchForCharacter(characterId, MarketOrder::Type::Sell)).first;

        return it->second;
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider::getBuyOrders(Character::IdType characterId) const
    {
        auto it = mBuyOrders.find(characterId);
        if (it == std::end(mBuyOrders))
            it = mBuyOrders.emplace(characterId, mOrderRepo.fetchForCharacter(characterId, MarketOrder::Type::Buy)).first;

        return it->second;
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider
    ::getArchivedOrders(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const
    {
        auto it = mArchivedOrders.find(characterId);
        if (it == std::end(mArchivedOrders))
            it = mArchivedOrders.emplace(characterId, mOrderRepo.fetchArchivedForCharacter(characterId)).first;

        std::vector<std::shared_ptr<MarketOrder>> result;
        for (const auto &order : it->second)
        {
            const auto lastSeen = order->getLastSeen();

            if (lastSeen >= from && lastSeen <= to)
                result.emplace_back(order);
        }

        return result;
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider::getSellOrdersForCorporation(quint64 corporationId) const
    {
        auto it = mCorpSellOrders.find(corporationId);
        if (it == std::end(mCorpSellOrders))
            it = mCorpSellOrders.emplace(corporationId, mOrderRepo.fetchForCorporation(corporationId, MarketOrder::Type::Sell)).first;

        return it->second;
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider::getBuyOrdersForCorporation(quint64 corporationId) const
    {
        auto it = mCorpBuyOrders.find(corporationId);
        if (it == std::end(mCorpBuyOrders))
            it = mCorpBuyOrders.emplace(corporationId, mOrderRepo.fetchForCorporation(corporationId, MarketOrder::Type::Buy)).first;

        return it->second;
    }

    MarketOrderProvider::OrderList CachingMarketOrderProvider
    ::getArchivedOrdersForCorporation(quint64 corporationId, const QDateTime &from, const QDateTime &to) const
    {
        auto it = mCorpArchivedOrders.find(corporationId);
        if (it == std::end(mCorpArchivedOrders))
            it = mCorpArchivedOrders.emplace(corporationId, mOrderRepo.fetchArchivedForCorporation(corporationId)).first;

        std::vector<std::shared_ptr<MarketOrder>> result;
        for (const auto &order : it->second)
        {
            const auto lastSeen = order->getLastSeen();

            if (lastSeen >= from && lastSeen <= to)
                result.emplace_back(order);
        }

        return result;
    }

    void CachingMarketOrderProvider::removeOrder(MarketOrder::IdType id)
    {
        clearAll();

        mOrderRepo.remove(id);

        emit orderChanged();
    }

    void CachingMarketOrderProvider::setOrderNotes(MarketOrder::IdType id, const QString &notes)
    {
        clearAll();

        mOrderRepo.setNotes(id, notes);

        emit orderChanged();
    }

    void CachingMarketOrderProvider::setOrderStation(MarketOrder::IdType orderId, uint stationId)
    {
        clearAll();

        mOrderRepo.setStation(orderId, stationId);

        emit orderChanged();
    }

    void CachingMarketOrderProvider::setOrderColorTag(MarketOrder::IdType orderId, const QColor &color)
    {
        clearAll();

        mOrderRepo.setColorTag(orderId, color);

        emit orderChanged();
    }

    void CachingMarketOrderProvider::clearOrdersForCharacter(Character::IdType id) const
    {
        mSellOrders.erase(id);
        mBuyOrders.erase(id);
        mArchivedOrders.erase(id);
    }

    void CachingMarketOrderProvider::clearOrdersForCorporation(uint id) const
    {
        mCorpSellOrders.erase(id);
        mCorpBuyOrders.erase(id);
        mCorpArchivedOrders.erase(id);
    }

    void CachingMarketOrderProvider::clearArchived() const
    {
        mArchivedOrders.clear();
        mCorpArchivedOrders.clear();
    }

    void CachingMarketOrderProvider::clearAll()
    {
        mSellOrders.clear();
        mBuyOrders.clear();
        mArchivedOrders.clear();
        mCorpSellOrders.clear();
        mCorpBuyOrders.clear();
        mCorpArchivedOrders.clear();
    }
}
