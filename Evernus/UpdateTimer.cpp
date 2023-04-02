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
#include "UpdateTimer.h"

namespace Evernus
{
    Character::IdType UpdateTimer::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void UpdateTimer::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    TimerType UpdateTimer::getType() const noexcept
    {
        return mType;
    }

    void UpdateTimer::setType(TimerType type) noexcept
    {
        mType = type;
    }

    QDateTime UpdateTimer::getUpdateTime() const
    {
        return mUpdateTime;
    }

    void UpdateTimer::setUpdateTime(const QDateTime &dt)
    {
        mUpdateTime = dt;
    }
}
