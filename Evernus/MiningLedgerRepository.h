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

#include "MiningLedger.h"
#include "Repository.h"
#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class MiningLedgerRepository
        : public Repository<MiningLedger>
    {
    public:
        using TypeQuantityMap = std::unordered_map<EveType::IdType, quint64>;
        using SolarSystemQuantityMap = std::unordered_map<uint, quint64>;

        struct SolarSystemTypeData
        {
            quint64 mQuantity = 0;
            EveType::IdType mTypeId = EveType::invalidId;
            uint mSolarSystemId = 0;
        };

        using SolarSystemTypeAggregatedData = std::vector<SolarSystemTypeData>;

        using Repository::Repository;
        MiningLedgerRepository(const MiningLedgerRepository &) = default;
        MiningLedgerRepository(MiningLedgerRepository &&) = default;
        virtual ~MiningLedgerRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create(const Repository<Character> &characterRepo) const;

        void removeForCharacter(Character::IdType characterId) const;

        EntityList fetchForCharacter(Character::IdType characterId,
                                     const QDate &from,
                                     const QDate &to) const;
        TypeQuantityMap fetchTypesForCharacter(Character::IdType characterId,
                                               const QDate &from,
                                               const QDate &to) const;
        SolarSystemQuantityMap fetchSolarSystemsForCharacter(Character::IdType characterId,
                                                             const QDate &from,
                                                             const QDate &to) const;
        SolarSystemTypeAggregatedData fetchAggregatedDataForCharacter(Character::IdType characterId,
                                                                      const QDate &from,
                                                                      const QDate &to) const;

        MiningLedgerRepository &operator =(const MiningLedgerRepository &) = default;
        MiningLedgerRepository &operator =(MiningLedgerRepository &&) = default;

    private:
        virtual QStringList getColumns() const override;
        virtual void bindValues(const MiningLedger &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const MiningLedger &entity, QSqlQuery &query) const override;
    };
}
