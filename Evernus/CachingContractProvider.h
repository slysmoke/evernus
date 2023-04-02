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

#include <unordered_map>

#include "ContractProvider.h"

namespace Evernus
{
    class ContractRepository;

    class CachingContractProvider
        : public ContractProvider
    {
    public:
        explicit CachingContractProvider(const ContractRepository &contractRepo);
        virtual ~CachingContractProvider() = default;

        virtual ContractList getIssuedContracts(Character::IdType characterId) const override;
        virtual ContractList getAssignedContracts(Character::IdType characterId) const override;
        virtual ContractList getIssuedContractsForCorporation(quint64 corporationId) const override;
        virtual ContractList getAssignedContractsForCorporation(quint64 corporationId) const override;

        void clearForCharacter(Character::IdType id) const;
        void clearForCorporation(uint id) const;

    private:
        const ContractRepository &mContractRepo;

        mutable std::unordered_map<Character::IdType, ContractList> mIssuedContracts, mAssignedContracts;
        mutable std::unordered_map<quint64, ContractList> mIssuedCorpContracts, mAssignedCorpContracts;
    };
}
