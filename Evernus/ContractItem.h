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

#include "EveType.h"

namespace Evernus
{
    class ContractItem
        : public Entity<quint64>
    {
    public:
        using Entity::Entity;

        ContractItem() = default;
        ContractItem(const ContractItem &) = default;
        ContractItem(ContractItem &&) = default;

        quint64 getContractId() const noexcept;
        void setContractId(quint64 id) noexcept;

        EveType::IdType getTypeId() const noexcept;
        void setTypeId(EveType::IdType id) noexcept;

        quint64 getQuantity() const noexcept;
        void setQuantity(quint64 value) noexcept;

        bool isIncluded() const noexcept;
        void setIncluded(bool flag) noexcept;

        ContractItem &operator =(const ContractItem &) = default;
        ContractItem &operator =(ContractItem &&) = default;

    private:
        quint64 mContractId = 0;
        EveType::IdType mTypeId = EveType::invalidId;
        quint64 mQuantity = 0;
        bool mIncluded = false;
    };
}
