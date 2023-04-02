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
#include "CorpWalletSnapshot.h"

namespace Evernus
{
    CorpWalletSnapshot::CorpWalletSnapshot(const IdType &id, double balance)
        : Entity{id}
        , mBalance{balance}
    {
    }

    QDateTime CorpWalletSnapshot::getTimestamp() const
    {
        return mTimestamp;
    }

    void CorpWalletSnapshot::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    quint64 CorpWalletSnapshot::getCorporationId() const noexcept
    {
        return mCorporationId;
    }

    void CorpWalletSnapshot::setCorporationId(quint64 id) noexcept
    {
        mCorporationId = id;
    }

    double CorpWalletSnapshot::getBalance() const noexcept
    {
        return mBalance;
    }

    void CorpWalletSnapshot::setBalance(double balance) noexcept
    {
        mBalance = balance;
    }
}
