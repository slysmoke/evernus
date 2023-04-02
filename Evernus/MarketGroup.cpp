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
#include "MarketGroup.h"

namespace Evernus
{
    MarketGroup::ParentIdType MarketGroup::getParentId() const
    {
        return mParentId;
    }

    void MarketGroup::setParentId(const ParentIdType &id)
    {
        mParentId = id;
    }

    QString MarketGroup::getName() const &
    {
        return mName;
    }

    QString &&MarketGroup::getName() && noexcept
    {
        return std::move(mName);
    }

    void MarketGroup::setName(const QString &name)
    {
        mName = name;
    }

    void MarketGroup::setName(QString &&name)
    {
        mName = std::move(name);
    }

    MarketGroup::DescriptionType MarketGroup::getDescription() const &
    {
        return mDescription;
    }

    MarketGroup::DescriptionType &&MarketGroup::getDescription() && noexcept
    {
        return std::move(mDescription);
    }

    void MarketGroup::setDescription(const DescriptionType &name)
    {
        mDescription = name;
    }

    void MarketGroup::setDescription(DescriptionType &&name)
    {
        mDescription = std::move(name);
    }

    MarketGroup::IconIdType MarketGroup::getIconId() const
    {
        return mIconId;
    }

    void MarketGroup::setIconId(const IconIdType &id)
    {
        mIconId = id;
    }

    bool MarketGroup::hasTypes() const noexcept
    {
        return mHasTypes;
    }

    void MarketGroup::setHasTypes(bool has)
    {
        mHasTypes = has;
    }
}
