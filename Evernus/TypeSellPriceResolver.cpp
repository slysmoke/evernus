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
#include <functional>

#include <boost/range/adaptor/filtered.hpp>

#include "TypeSellPriceResolver.h"
#include "ExternalOrder.h"

namespace Evernus
{
    void TypeSellPriceResolver::setOrders(OrderList orders)
    {
        mOrders = std::move(orders);
        refreshPrices();
    }

    void TypeSellPriceResolver::setSellPriceType(PriceType type)
    {
        mSellPriceType = type;
        refreshPrices();
    }

    void TypeSellPriceResolver::setSellStation(quint64 stationId)
    {
        mSellStation = stationId;
        refreshPrices();
    }

    void TypeSellPriceResolver::refreshPrices()
    {
        mPrices.clear();

        if (mOrders)
        {
            const auto orderFilter = [=](const auto &order) {
                return (order.getType() == mSellPriceType) && (mSellStation == 0 || mSellStation == order.getStationId());
            };

            std::function<bool(const ExternalOrder &, double)> isBetter;
            if (mSellPriceType == PriceType::Buy)
                isBetter = [](const auto &order, auto price) { return order.getPrice() > price; };
            else
                isBetter = [](const auto &order, auto price) { return order.getPrice() < price; };

            for (const auto &order : *mOrders | boost::adaptors::filtered(orderFilter))
            {
                const auto typeId = order.getTypeId();
                const auto existingPrice = mPrices.find(typeId);

                if (existingPrice == std::end(mPrices))
                    mPrices[typeId] = order.getPrice();
                else if (isBetter(order, existingPrice->second))
                    mPrices[order.getTypeId()] = order.getPrice();
            }
        }
    }

    double TypeSellPriceResolver::getPrice(EveType::IdType typeId, quint64 quantity) const
    {
        const auto price = mPrices.find(typeId);
        return (price == std::end(mPrices)) ? (0.) : (price->second * quantity);
    }
}
