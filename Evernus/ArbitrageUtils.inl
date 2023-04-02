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
#include "ExternalOrder.h"

namespace Evernus
{
    namespace ArbitrageUtils
    {
        template<class Orders>
        std::vector<UsedOrder> fillOrders(Orders &orders, uint volume, bool requireVolume)
        {
            std::vector<UsedOrder> usedOrders;
            for (auto &order : orders)
            {
                const auto orderVolume = order.getVolumeRemaining();
                if (volume >= order.getMinVolume() && orderVolume > 0)
                {
                    const auto amount = std::min(orderVolume, volume);
                    volume -= amount;

                    // NOTE: we're casting away const, but not modifying the actual set key
                    // looks dirty, but there's no partial constness
                    const_cast<ExternalOrder &>(order).setVolumeRemaining(orderVolume - amount);

                    UsedOrder used{amount, order.getPrice()};
                    usedOrders.emplace_back(used);

                    if (volume == 0)
                        return usedOrders;
                }
            }

            return (requireVolume) ? (std::vector<UsedOrder>{}) : (usedOrders);
        }
    }
}
