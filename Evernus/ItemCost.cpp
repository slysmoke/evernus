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
#include <QSettings>

#include "PriceSettings.h"

#include "ItemCost.h"

namespace Evernus
{
    Character::IdType ItemCost::getCharacterId() const
    {
        return mCharacterId;
    }

    void ItemCost::setCharacterId(Character::IdType id)
    {
        mCharacterId = id;
    }

    EveType::IdType ItemCost::getTypeId() const
    {
        return mTypeId;
    }

    void ItemCost::setTypeId(EveType::IdType id)
    {
        mTypeId = id;
    }

    double ItemCost::getAdjustedCost() const noexcept
    {
        QSettings settings;
        return
            mCost *
            (settings.value(PriceSettings::itemRelativeCostAddKey, PriceSettings::itemRelativeCostAddDefault).toDouble() / 100. + 1.) +
            settings.value(PriceSettings::itemConstCostAddKey, PriceSettings::itemConstCostAddDefault).toDouble();
    }

    double ItemCost::getCost() const noexcept
    {
        return mCost;
    }

    void ItemCost::setCost(double cost) noexcept
    {
        mCost = cost;
    }
}
