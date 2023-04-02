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

#include "Character.h"

namespace Evernus
{
    class AssetValueSnapshot
        : public Entity<uint>
    {
    public:
        using Entity::Entity;

        AssetValueSnapshot() = default;
        AssetValueSnapshot(const IdType &id, double balance);
        AssetValueSnapshot(const AssetValueSnapshot &) = default;
        AssetValueSnapshot(AssetValueSnapshot &&) = default;
        virtual ~AssetValueSnapshot() = default;

        QDateTime getTimestamp() const;
        void setTimestamp(const QDateTime &dt);

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        double getBalance() const noexcept;
        void setBalance(double balance) noexcept;

        AssetValueSnapshot &operator =(const AssetValueSnapshot &) = default;
        AssetValueSnapshot &operator =(AssetValueSnapshot &&) = default;

    private:
        QDateTime mTimestamp;
        Character::IdType mCharacterId = Character::invalidId;
        double mBalance = 0.;
    };
}
