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
    class MetaGroup
        : public Entity<uint>
    {
    public:
        typedef std::optional<QString> DescriptionType;
        typedef std::optional<uint> RaceIdType;

        using Entity::Entity;

        MetaGroup() = default;
        MetaGroup(const MetaGroup &) = default;
        MetaGroup(MetaGroup &&) = default;
        virtual ~MetaGroup() = default;

        QString getName() const &;
        QString &&getName() && noexcept;
        void setName(const QString &name);
        void setName(QString &&name);

        DescriptionType getDescription() const;
        void setDescription(const DescriptionType &desc);
        MetaGroup &operator =(const MetaGroup &) = default;
        MetaGroup &operator =(MetaGroup &&) = default;

    private:
        QString mName;
        DescriptionType mDescription;
    };
}
