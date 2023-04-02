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
#include "CorpAssetValueSnapshot.h"

namespace Evernus
{
    CorpAssetValueSnapshot::CorpAssetValueSnapshot(const IdType &id, double balance)
        : Entity{id}
        , mBalance{balance}
    {
    }

    QDateTime CorpAssetValueSnapshot::getTimestamp() const
    {
        return mTimestamp;
    }

    void CorpAssetValueSnapshot::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    quint64 CorpAssetValueSnapshot::getCorporationId() const noexcept
    {
        return mCorporationId;
    }

    void CorpAssetValueSnapshot::setCorporationId(quint64 id) noexcept
    {
        mCorporationId = id;
    }

    double CorpAssetValueSnapshot::getBalance() const noexcept
    {
        return mBalance;
    }

    void CorpAssetValueSnapshot::setBalance(double balance) noexcept
    {
        mBalance = balance;
    }
}
