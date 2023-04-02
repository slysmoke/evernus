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

#include <optional>

#include <QString>

#include "Entity.h"

namespace Evernus
{
    class MarketGroup
        : public Entity<uint>
    {
    public:
        typedef std::optional<IdType> ParentIdType;
        typedef std::optional<QString> DescriptionType;
        typedef std::optional<uint> IconIdType;

        using Entity::Entity;

        MarketGroup() = default;
        MarketGroup(const MarketGroup &) = default;
        MarketGroup(MarketGroup &&) = default;
        virtual ~MarketGroup() = default;

        ParentIdType getParentId() const;
        void setParentId(const ParentIdType &id);

        QString getName() const &;
        QString &&getName() && noexcept;
        void setName(const QString &name);
        void setName(QString &&name);

        DescriptionType getDescription() const &;
        DescriptionType &&getDescription() && noexcept;
        void setDescription(const DescriptionType &name);
        void setDescription(DescriptionType &&name);

        IconIdType getIconId() const;
        void setIconId(const IconIdType &id);

        bool hasTypes() const noexcept;
        void setHasTypes(bool has);

        MarketGroup &operator =(const MarketGroup &) = default;
        MarketGroup &operator =(MarketGroup &&) = default;

    private:
        ParentIdType mParentId;
        QString mName;
        DescriptionType mDescription;
        IconIdType mIconId;
        bool mHasTypes = true;
    };
}
