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
#include "MiningLedger.h"

namespace Evernus
{
    Character::IdType MiningLedger::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void MiningLedger::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    QDate MiningLedger::getDate() const
    {
        return mDate;
    }

    void MiningLedger::setDate(const QDate &date)
    {
        mDate = date;
    }

    uint MiningLedger::getQuantity() const noexcept
    {
        return mQuantity;
    }

    void MiningLedger::setQuantity(uint value) noexcept
    {
        mQuantity = value;
    }

    uint MiningLedger::getSolarSystemId() const noexcept
    {
        return mSolarSystemId;
    }

    void MiningLedger::setSolarSystemId(uint id) noexcept
    {
        mSolarSystemId = id;
    }

    EveType::IdType MiningLedger::getTypeId() const noexcept
    {
        return mTypeId;
    }

    void MiningLedger::setTypeId(EveType::IdType id) noexcept
    {
        mTypeId = id;
    }
}
