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
#pragma once

#include <vector>

#include <QtGlobal>

namespace Evernus
{
    namespace ArbitrageUtils
    {
        struct UsedOrder
        {
            uint mVolume;
            double mPrice;
        };

        template<class Orders>
        std::vector<UsedOrder> fillOrders(Orders &orders, uint volume, bool requireVolume);

        double getStationTax(double corpStanding) noexcept;
        double getReprocessingTax(const std::vector<UsedOrder> &orders, double stationTax, uint desiredVolume) noexcept;
    }
}

#include "ArbitrageUtils.inl"
