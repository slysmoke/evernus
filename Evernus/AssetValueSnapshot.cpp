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
#include "AssetValueSnapshot.h"

namespace Evernus
{
    AssetValueSnapshot::AssetValueSnapshot(const IdType &id, double balance)
        : Entity{id}
        , mBalance{balance}
    {
    }

    QDateTime AssetValueSnapshot::getTimestamp() const
    {
        return mTimestamp;
    }

    void AssetValueSnapshot::setTimestamp(const QDateTime &dt)
    {
        mTimestamp = dt;
    }

    Character::IdType AssetValueSnapshot::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void AssetValueSnapshot::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    double AssetValueSnapshot::getBalance() const noexcept
    {
        return mBalance;
    }

    void AssetValueSnapshot::setBalance(double balance) noexcept
    {
        mBalance = balance;
    }
}
