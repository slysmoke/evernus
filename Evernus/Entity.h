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

namespace Evernus
{
    template<class Id>
    class Entity
    {
    public:
        using IdType = Id;

        static const IdType invalidId;

        Entity() = default;
        Entity(const IdType &id);
        Entity(IdType &&id);
        Entity(const Entity &other) = default;
        Entity(Entity &&other) = default;
        virtual ~Entity() = default;

        IdType getId() const;
        void setId(IdType id);

        IdType getOriginalId() const;
        void updateOriginalId();

        bool isNew() const noexcept;
        void setNew(bool isNew) noexcept;

        Entity &operator =(const Entity &other) = default;
        Entity &operator =(Entity &&other) = default;

    private:
        IdType mId = invalidId, mOriginalId = invalidId;

        bool mIsNew = true;
    };
}

#include "Entity.inl"
