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
#include "CharacterRepository.h"
#include "ContractProvider.h"

#include "AssignedContractModel.h"

namespace Evernus
{
    AssignedContractModel::AssignedContractModel(const EveDataProvider &dataProvider,
                                                 const ContractProvider &contractProvider,
                                                 const CharacterRepository &characterRepo,
                                                 bool corp,
                                                 QObject *parent)
        : ContractModel{dataProvider, parent}
        , mContractProvider{contractProvider}
        , mCharacterRepo{characterRepo}
        , mCorp{corp}
    {
    }

    void AssignedContractModel::setCharacter(Character::IdType id)
    {
        mCharacterId = id;

        try
        {
            mCorpId = mCharacterRepo.getCorporationId(mCharacterId);
        }
        catch (const CharacterRepository::NotFoundException &)
        {
            mCorpId = 0;
        }

        reset();
    }

    ContractModel::ContractList AssignedContractModel::getContracts() const
    {
        return (mCorp) ?
               (mContractProvider.getAssignedContractsForCorporation(mCorpId)) :
               (mContractProvider.getAssignedContracts(mCharacterId));
    }
}
