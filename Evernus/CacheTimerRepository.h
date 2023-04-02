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
#include "CacheTimer.h"

namespace Evernus
{
    class CacheTimerRepository
        : public Repository<CacheTimer>
    {
    public:
        using Repository::Repository;
        virtual ~CacheTimerRepository() = default;

        virtual QString getTableName() const override;
        virtual QString getIdColumn() const override;

        virtual EntityPtr populate(const QSqlRecord &record) const override;

        void create(const Repository<Character> &characterRepo) const;

    private:
        virtual QStringList getColumns() const override;
        virtual void bindValues(const CacheTimer &entity, QSqlQuery &query) const override;
        virtual void bindPositionalValues(const CacheTimer &entity, QSqlQuery &query) const override;
    };
}
