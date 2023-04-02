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
#include "ContractItem.h"

namespace Evernus
{
    quint64 ContractItem::getContractId() const noexcept
    {
        return mContractId;
    }

    void ContractItem::setContractId(quint64 id) noexcept
    {
        mContractId = id;
    }

    EveType::IdType ContractItem::getTypeId() const noexcept
    {
        return mTypeId;
    }

    void ContractItem::setTypeId(EveType::IdType id) noexcept
    {
        mTypeId = id;
    }

    quint64 ContractItem::getQuantity() const noexcept
    {
        return mQuantity;
    }

    void ContractItem::setQuantity(quint64 value) noexcept
    {
        mQuantity = value;
    }

    bool ContractItem::isIncluded() const noexcept
    {
        return mIncluded;
    }

    void ContractItem::setIncluded(bool flag) noexcept
    {
        mIncluded = flag;
    }
}
