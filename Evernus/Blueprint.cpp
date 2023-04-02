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
#include "Blueprint.h"

namespace Evernus
{
    quint64 Blueprint::getId() const noexcept
    {
        return mId;
    }

    void Blueprint::setId(quint64 value) noexcept
    {
        mId = value;
    }

    Blueprint::Location Blueprint::getLocation() const noexcept
    {
        return mLocation;
    }

    void Blueprint::setLocation(Location location) noexcept
    {
        mLocation = location;
    }

    quint64 Blueprint::getLocationId() const noexcept
    {
        return mLocationId;
    }

    void Blueprint::setLocationId(quint64 value) noexcept
    {
        mLocationId = value;
    }

    uint Blueprint::getMaterialEfficiency() const noexcept
    {
        return mMaterialEfficiency;
    }

    void Blueprint::setMaterialEfficiency(uint value) noexcept
    {
        mMaterialEfficiency = value;
    }

    uint Blueprint::getTimeEfficiency() const noexcept
    {
        return mTimeEfficiency;
    }

    void Blueprint::setTimeEfficiency(uint value) noexcept
    {
        mTimeEfficiency = value;
    }

    int Blueprint::getQuantity() const noexcept
    {
        return mQuantity;
    }

    void Blueprint::setQuantity(int value) noexcept
    {
        mQuantity = value;
    }

    int Blueprint::getRuns() const noexcept
    {
        return mRuns;
    }

    void Blueprint::setRuns(int value) noexcept
    {
        mRuns = value;
    }

    EveType::IdType Blueprint::getTypeId() const noexcept
    {
        return mTypeId;
    }

    void Blueprint::setTypeId(EveType::IdType id) noexcept
    {
        mTypeId = id;
    }

    bool Blueprint::isBPC() const noexcept
    {
        return mQuantity == magicBPCQuantity;
    }
}
