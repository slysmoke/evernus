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
#include <map>

#include "MarketHistoryEntry.h"
#include "EveType.h"

namespace Evernus
{
    class ExternalOrder;

    class MarketDataProvider
    {
    public:
        template<class T>
        using TypeMap = std::unordered_map<EveType::IdType, T>;
        using HistoryMap = TypeMap<std::map<QDate, MarketHistoryEntry>>;
        using HistoryRegionMap = std::unordered_map<uint, HistoryMap>;
        using OrderResultType = std::vector<ExternalOrder>;

        MarketDataProvider() = default;
        MarketDataProvider(const MarketDataProvider &) = default;
        MarketDataProvider(MarketDataProvider &&) = default;
        virtual ~MarketDataProvider() = default;

        virtual const HistoryMap *getHistory(uint regionId) const = 0;
        virtual const HistoryRegionMap *getHistory() const = 0;
        virtual const OrderResultType *getOrders() const = 0;

        MarketDataProvider &operator =(const MarketDataProvider &) = default;
        MarketDataProvider &operator =(MarketDataProvider &&) = default;
    };
}
