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
#include <vector>
#include <memory>

#include "PriceType.h"
#include "EveType.h"

namespace Evernus
{
    class ExternalOrder;

    class TypeSellPriceResolver final
    {
    public:
        using OrderList = std::shared_ptr<std::vector<ExternalOrder>>;

        TypeSellPriceResolver() = default;
        TypeSellPriceResolver(const TypeSellPriceResolver &) = default;
        TypeSellPriceResolver(TypeSellPriceResolver &&) = default;
        ~TypeSellPriceResolver() = default;

        void setOrders(OrderList orders);
        void setSellPriceType(PriceType type);
        void setSellStation(quint64 stationId);

        void refreshPrices();

        double getPrice(EveType::IdType typeId, quint64 quantity) const;

        TypeSellPriceResolver &operator =(const TypeSellPriceResolver &) = default;
        TypeSellPriceResolver &operator =(TypeSellPriceResolver &&) = default;

    private:
        OrderList mOrders;
        std::unordered_map<EveType::IdType, double> mPrices;

        PriceType mSellPriceType = PriceType::Sell;
        quint64 mSellStation = 0;
    };
}
