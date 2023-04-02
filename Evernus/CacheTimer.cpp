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
#include "CacheTimer.h"

namespace Evernus
{
    Character::IdType CacheTimer::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void CacheTimer::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    TimerType CacheTimer::getType() const noexcept
    {
        return mType;
    }

    void CacheTimer::setType(TimerType type) noexcept
    {
        mType = type;
    }

    QDateTime CacheTimer::getCacheUntil() const
    {
        return mCacheUntil;
    }

    void CacheTimer::setCacheUntil(const QDateTime &dt)
    {
        mCacheUntil = dt;
    }
}
