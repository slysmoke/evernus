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
#include <memory>

#include "MarketOrder.h"

class QDateTime;
class QColor;

namespace Evernus
{
    class MarketOrderProvider
    {
    public:
        typedef std::vector<std::shared_ptr<MarketOrder>> OrderList;

        MarketOrderProvider() = default;
        virtual ~MarketOrderProvider() = default;

        virtual OrderList getSellOrders(Character::IdType characterId) const = 0;
        virtual OrderList getBuyOrders(Character::IdType characterId) const = 0;
        virtual OrderList getArchivedOrders(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const = 0;

        virtual OrderList getSellOrdersForCorporation(quint64 corporationId) const = 0;
        virtual OrderList getBuyOrdersForCorporation(quint64 corporationId) const = 0;
        virtual OrderList getArchivedOrdersForCorporation(quint64 corporationId, const QDateTime &from, const QDateTime &to) const = 0;

        virtual void removeOrder(MarketOrder::IdType id) = 0;

        virtual void setOrderNotes(MarketOrder::IdType id, const QString &notes) = 0;
        virtual void setOrderStation(MarketOrder::IdType orderId, uint stationId) = 0;
        virtual void setOrderColorTag(MarketOrder::IdType orderId, const QColor &color) = 0;
    };
}
