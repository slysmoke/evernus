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
#include "MetaGroup.h"

namespace Evernus
{
    QString MetaGroup::getName() const &
    {
        return mName;
    }

    QString &&MetaGroup::getName() && noexcept
    {
        return std::move(mName);
    }

    void MetaGroup::setName(const QString &name)
    {
        mName = name;
    }

    void MetaGroup::setName(QString &&name)
    {
        mName = std::move(name);
    }

    MetaGroup::DescriptionType MetaGroup::getDescription() const
    {
        return mDescription;
    }

    void MetaGroup::setDescription(const DescriptionType &desc)
    {
        mDescription = desc;
    }
}
