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

#include <QSqlDatabase>

namespace Evernus
{
    class DatabaseConnectionProvider;

    template<class T>
    class Repository
    {
    public:
        struct NotFoundException : std::exception { };

        typedef std::shared_ptr<T> EntityPtr;
        typedef std::vector<EntityPtr> EntityList;

        explicit Repository(const DatabaseConnectionProvider &connectionProvider);
        virtual ~Repository() = default;

        virtual QString getTableName() const = 0;
        virtual QString getIdColumn() const = 0;

        virtual EntityPtr populate(const QSqlRecord &record) const = 0;

        QSqlQuery exec(const QString &query) const;
        QSqlQuery prepare(const QString &queryStr) const;
        void store(T &entity) const;

        template<class U>
        void batchStore(const U &entities, bool hasId, bool wrapIntransaction = true) const;

        template<class Id>
        void remove(Id &&id) const;

        EntityList fetchAll() const;

        template<class Id>
        EntityPtr find(Id &&id) const;

        QSqlDatabase getDatabase() const;

        typename T::IdType getLastInsertId() const;

    protected:
        const size_t maxSqliteBoundVariables = 999;

    private:
        const DatabaseConnectionProvider &mConnectionProvider;

        void insert(T &entity) const;
        void update(const T &entity) const;

        virtual QStringList getColumns() const = 0;
        virtual void bindValues(const T &entity, QSqlQuery &query) const = 0;
        virtual void bindPositionalValues(const T &entity, QSqlQuery &query) const = 0;

        virtual void preStore(T &entity) const;
        virtual void postStore(T &entity) const;

        size_t getMaxRowsPerInsert() const;
    };
}

#include "Repository.inl"
