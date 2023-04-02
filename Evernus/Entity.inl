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
#include <utility>

namespace Evernus
{
    template<class Id>
    const typename Entity<Id>::IdType Entity<Id>::invalidId = IdType{};

    template<class Id>
    Entity<Id>::Entity(const IdType &id)
        : mId{id}
        , mOriginalId{id}
    {
    }

    template<class Id>
    Entity<Id>::Entity(IdType &&id)
        : mId{id}
        , mOriginalId{std::move(id)}
    {
    }

    template<class Id>
    typename Entity<Id>::IdType Entity<Id>::getId() const
    {
        return mId;
    }

    template<class Id>
    void Entity<Id>::setId(IdType id)
    {
        mId = id;
    }

    template<class Id>
    typename Entity<Id>::IdType Entity<Id>::getOriginalId() const
    {
        return mOriginalId;
    }

    template<class Id>
    void Entity<Id>::updateOriginalId()
    {
        mOriginalId = mId;
    }

    template<class Id>
    bool Entity<Id>::isNew() const noexcept
    {
        return mIsNew;
    }

    template<class Id>
    void Entity<Id>::setNew(bool isNew) noexcept
    {
        mIsNew = isNew;
    }
}
