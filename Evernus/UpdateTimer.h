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
#include <QString>

#include "TimerTypes.h"
#include "Character.h"
#include "Entity.h"

namespace Evernus
{
    class UpdateTimer
        : public Entity<uint>
    {
    public:
        using Entity::Entity;

        UpdateTimer() = default;
        UpdateTimer(const UpdateTimer &) = default;
        UpdateTimer(UpdateTimer &&) = default;
        virtual ~UpdateTimer() = default;

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        TimerType getType() const noexcept;
        void setType(TimerType type) noexcept;

        QDateTime getUpdateTime() const;
        void setUpdateTime(const QDateTime &dt);

        UpdateTimer &operator =(const UpdateTimer &) = default;
        UpdateTimer &operator =(UpdateTimer &&) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        TimerType mType = TimerType::Character;
        QDateTime mUpdateTime;
    };
}
