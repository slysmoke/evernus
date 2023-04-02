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

#include <QDate>

#include "Character.h"
#include "EveType.h"
#include "Entity.h"

namespace Evernus
{
    class MiningLedger
        : public Entity<quint64>
    {
    public:
        using Entity::Entity;

        MiningLedger() = default;
        MiningLedger(const MiningLedger &) = default;
        MiningLedger(MiningLedger &&) = default;
        virtual ~MiningLedger() = default;

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        QDate getDate() const;
        void setDate(const QDate &date);

        uint getQuantity() const noexcept;
        void setQuantity(uint value) noexcept;

        uint getSolarSystemId() const noexcept;
        void setSolarSystemId(uint id) noexcept;

        EveType::IdType getTypeId() const noexcept;
        void setTypeId(EveType::IdType id) noexcept;

        MiningLedger &operator =(const MiningLedger &) = default;
        MiningLedger &operator =(MiningLedger &&) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        QDate mDate;
        uint mQuantity = 0;
        uint mSolarSystemId = 0;
        EveType::IdType mTypeId = EveType::invalidId;
    };
}
