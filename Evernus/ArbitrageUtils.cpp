/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for mScrapmetal details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <algorithm>

#include "ArbitrageUtils.h"

namespace Evernus
{
    namespace ArbitrageUtils
    {
        double getStationTax(double corpStanding) noexcept
        {
            return std::max(0., 5. - corpStanding * 0.75) / 100.;
        }

        double getReprocessingTax(const std::vector<UsedOrder> &orders, double stationTax, uint desiredVolume) noexcept
        {
            // eve uses it's adjusted prices, but let's use the actual order prices for calculations
            uint totalVolume = 0;
            double totalPrice = 0.;
            double totalCost = 0.;

            for (const auto &order : orders)
            {
                totalVolume += order.mVolume;
                totalCost += order.mVolume * order.mPrice;
                totalPrice += order.mPrice;
            }

            Q_ASSERT(desiredVolume >= totalVolume);

            // assume remaining volume would sell for avg price
            const auto remainingVolume = desiredVolume - totalVolume;
            totalCost += remainingVolume * totalPrice / orders.size();

            return stationTax * totalCost;
        }
    }
}
