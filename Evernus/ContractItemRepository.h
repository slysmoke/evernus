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

#include "ContractItem.h"
#include "Repository.h"
#include "Contract.h"

namespace Evernus
{
    class ContractItemRepository
        : public Repository<ContractItem>
    {
    public:
        ContractItemRepository(bool corp, const DatabaseConnectionProvider &connectionProvider);
        virtual ~ContractItemRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;
        EntityPtr populate(const QString &prefix, const QSqlRecord &record) const;

        void create(const Repository<Contract> &contractRepo) const;

        void deleteForContract(Contract::IdType id) const;

        virtual QStringList getColumns() const override;

    private:
        bool mCorp = false;

        virtual void bindValues(const ContractItem &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const ContractItem &entity, QSqlQuery &query) const override;
    };
}
