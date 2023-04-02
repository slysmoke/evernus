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

#include <vector>
#include <memory>

#include "Character.h"

namespace Evernus
{
    class Contract;

    class ContractProvider
    {
    public:
        typedef std::vector<std::shared_ptr<Contract>> ContractList;

        ContractProvider() = default;
        virtual ~ContractProvider() = default;

        virtual ContractList getIssuedContracts(Character::IdType characterId) const = 0;
        virtual ContractList getAssignedContracts(Character::IdType characterId) const = 0;
        virtual ContractList getIssuedContractsForCorporation(quint64 corporationId) const = 0;
        virtual ContractList getAssignedContractsForCorporation(quint64 corporationId) const = 0;
    };
}
