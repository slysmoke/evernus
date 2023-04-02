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

#include "Repository.h"
#include "ItemCost.h"

namespace Evernus
{
    class ItemCostRepository
        : public Repository<ItemCost>
    {
    public:
        using Repository::Repository;
        virtual ~ItemCostRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create(const Repository<Character> &characterRepo) const;

        EntityList fetchForCharacter(Character::IdType id) const;
        EntityPtr fetchForCharacterAndType(Character::IdType characterId, EveType::IdType typeId) const;
        EntityPtr fetchLatestForType(EveType::IdType typeId) const;

        void removeForCharacter(Character::IdType characterId) const;

    private:
        virtual QStringList getColumns() const override;
        virtual void bindValues(const ItemCost &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const ItemCost &entity, QSqlQuery &query) const override;
    };
}
