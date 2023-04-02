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
#pragma once

#include <QDateTime>

#include "TimerTypes.h"
#include "Character.h"

namespace Evernus
{
    class CacheTimerProvider
    {
    public:
        CacheTimerProvider() = default;
        virtual ~CacheTimerProvider() = default;

        virtual QDateTime getLocalCacheTimer(Character::IdType id, TimerType type) const = 0;
        virtual void setUtcCacheTimer(Character::IdType id, TimerType type, const QDateTime &dt) = 0;

        virtual QDateTime getLocalUpdateTimer(Character::IdType id, TimerType type) const = 0;
    };
}
