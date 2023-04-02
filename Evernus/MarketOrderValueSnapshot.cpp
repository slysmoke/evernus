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
#include "MarketOrderValueSnapshot.h"

namespace Evernus
{
    QDateTime MarketOrderValueSnapshot::getTimestamp() const
    {
        return mTimestamp;
    }

    void MarketOrderValueSnapshot::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    Character::IdType MarketOrderValueSnapshot::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void MarketOrderValueSnapshot::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    double MarketOrderValueSnapshot::getBuyValue() const noexcept
    {
        return mBuyValue;
    }

    void MarketOrderValueSnapshot::setBuyValue(double value) noexcept
    {
        mBuyValue = value;
    }

    double MarketOrderValueSnapshot::getSellValue() const noexcept
    {
        return mSellValue;
    }

    void MarketOrderValueSnapshot::setSellValue(double value) noexcept
    {
        mSellValue = value;
    }
}
