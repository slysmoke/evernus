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
#include <algorithm>
#include <limits>
#include <cmath>

#include "EveDataProvider.h"

namespace Evernus::MathUtils
{
    template<class T>
    double calcPercentile(const T &orders,
                          quint64 maxVolume,
                          double avgPrice,
                          bool discardBogusOrders,
                          double bogusOrderThreshold)
    {
        if (orders.empty())
            return (std::isnan(avgPrice)) ? (0.) : (avgPrice);

        if (maxVolume == 0)
            maxVolume = 1;

        const auto nullAvg = qFuzzyIsNull(avgPrice);

        auto it = std::begin(orders);
        quint64 volume = 0u;
        auto result = 0.;

        while (volume < maxVolume && it != std::end(orders))
        {
            const auto price = it->get().getPrice();
            const quint64 orderVolume = it->get().getVolumeRemaining();
            const auto add = std::min(orderVolume, maxVolume - volume);

            if (!discardBogusOrders || nullAvg || fabs((price - avgPrice) / avgPrice) < bogusOrderThreshold)
            {
                volume += add;
                result += price * add;
            }
            else if (!nullAvg)
            {
                maxVolume -= add;
            }

            ++it;
        }

        if (maxVolume == 0) // all bogus orders?
            return std::begin(orders)->get().getPrice();

        return result / maxVolume;
    }

    template<class T>
    std::size_t batchSize(T value) noexcept
    {
        return std::max(static_cast<std::size_t>(1u), std::min(static_cast<std::size_t>(std::exp(std::log(value) / 1.38629436112 - 1.)), static_cast<std::size_t>(300u)));
    }

    template<class T>
    AggregateData calcAggregates(const T &orders, const EveDataProvider &dataProvider)
    {
        AggregateData result;

        result.mMinPrice = std::numeric_limits<double>::max();

        std::vector<double> prices;
        prices.reserve(orders.size());

        for (const auto &order : orders)
        {
            const auto price = order->getPrice();
            if (price < result.mMinPrice)
                result.mMinPrice = price;
            if (price > result.mMaxPrice)
                result.mMaxPrice = price;

            prices.emplace_back(price);

            const auto volume = order->getVolumeRemaining();

            result.mTotalPrice += price * volume;
            result.mTotalSize += dataProvider.getTypeVolume(order->getTypeId()) * volume;
            result.mTotalVolume += volume;
        }

        if (!prices.empty())
        {
            std::nth_element(std::begin(prices), std::next(std::begin(prices), prices.size() / 2), std::end(prices));
            result.mMedianPrice = prices[prices.size() / 2];
        }

        if (result.mMinPrice == std::numeric_limits<double>::max())
            result.mMinPrice = 0.;

        return result;
    }
}
