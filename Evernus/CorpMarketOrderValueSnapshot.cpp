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
#include "CorpMarketOrderValueSnapshot.h"

namespace Evernus
{
    QDateTime CorpMarketOrderValueSnapshot::getTimestamp() const
    {
        return mTimestamp;
    }

    void CorpMarketOrderValueSnapshot::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    quint64 CorpMarketOrderValueSnapshot::getCorporationId() const noexcept
    {
        return mCorporationId;
    }

    void CorpMarketOrderValueSnapshot::setCorporationId(quint64 id) noexcept
    {
        mCorporationId = id;
    }

    double CorpMarketOrderValueSnapshot::getBuyValue() const noexcept
    {
        return mBuyValue;
    }

    void CorpMarketOrderValueSnapshot::setBuyValue(double value) noexcept
    {
        mBuyValue = value;
    }

    double CorpMarketOrderValueSnapshot::getSellValue() const noexcept
    {
        return mSellValue;
    }

    void CorpMarketOrderValueSnapshot::setSellValue(double value) noexcept
    {
        mSellValue = value;
    }
}
