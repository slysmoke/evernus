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
#include "Contract.h"

namespace Evernus
{
    class ContractItemRepository;

    class ContractRepository
        : public Repository<Contract>
    {
    public:
        ContractRepository(const ContractItemRepository &contractItemRepo, bool corp, const DatabaseConnectionProvider &connectionProvider);
        virtual ~ContractRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create() const;

        EntityList fetchIssuedForCharacter(Character::IdType id) const;
        EntityList fetchAssignedForCharacter(Character::IdType id) const;

        EntityList fetchIssuedForCorporation(quint64 id) const;
        EntityList fetchAssignedForCorporation(quint64 id) const;

    private:
        const ContractItemRepository &mContractItemRepo;
        bool mCorp = false;

        virtual QStringList getColumns() const override;
        virtual void bindValues(const Contract &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const Contract &entity, QSqlQuery &query) const override;

        virtual void preStore(Contract &entity) const override;

        template<class T>
        EntityList fetchByColumnWithItems(T id, const QString &column) const;
    };
}
