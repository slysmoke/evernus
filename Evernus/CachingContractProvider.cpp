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
#include "ContractRepository.h"

#include "CachingContractProvider.h"

namespace Evernus
{
    CachingContractProvider::CachingContractProvider(const ContractRepository &contractRepo)
        : ContractProvider{}
        , mContractRepo{contractRepo}
    {
    }

    ContractProvider::ContractList CachingContractProvider::getIssuedContracts(Character::IdType characterId) const
    {
        const auto it = mIssuedContracts.find(characterId);
        if (it != std::end(mIssuedContracts))
            return it->second;

        return mIssuedContracts.emplace(characterId, mContractRepo.fetchIssuedForCharacter(characterId)).first->second;
    }

    ContractProvider::ContractList CachingContractProvider::getAssignedContracts(Character::IdType characterId) const
    {
        const auto it = mAssignedContracts.find(characterId);
        if (it != std::end(mAssignedContracts))
            return it->second;

        return mAssignedContracts.emplace(characterId, mContractRepo.fetchAssignedForCharacter(characterId)).first->second;
    }

    ContractProvider::ContractList CachingContractProvider::getIssuedContractsForCorporation(quint64 corporationId) const
    {
        const auto it = mIssuedCorpContracts.find(corporationId);
        if (it != std::end(mIssuedCorpContracts))
            return it->second;

        return mIssuedCorpContracts.emplace(corporationId, mContractRepo.fetchIssuedForCorporation(corporationId)).first->second;
    }

    ContractProvider::ContractList CachingContractProvider::getAssignedContractsForCorporation(quint64 corporationId) const
    {
        const auto it = mAssignedCorpContracts.find(corporationId);
        if (it != std::end(mAssignedCorpContracts))
            return it->second;

        return mAssignedCorpContracts.emplace(corporationId, mContractRepo.fetchAssignedForCorporation(corporationId)).first->second;
    }

    void CachingContractProvider::clearForCharacter(Character::IdType id) const
    {
        mIssuedContracts.erase(id);
        mAssignedContracts.erase(id);
    }

    void CachingContractProvider::clearForCorporation(uint id) const
    {
        mIssuedCorpContracts.erase(id);
        mAssignedCorpContracts.erase(id);
    }
}
