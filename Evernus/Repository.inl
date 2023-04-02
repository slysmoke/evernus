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
#include <stdexcept>

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

#include <QtDebug>

#include "DatabaseConnectionProvider.h"
#include "DatabaseUtils.h"

namespace Evernus
{
    template<class T>
    Repository<T>::Repository(const DatabaseConnectionProvider &connectionProvider)
        : mConnectionProvider{connectionProvider}
    {
    }

    template<class T>
    QSqlQuery Repository<T>::exec(const QString &query) const
    {
        qDebug() << "SQL:" << query;

        const auto db = getDatabase();

        auto result = db.exec(query);
        const auto error = db.lastError();

        if (error.isValid())
        {
            const auto errorText = error.text();

            qCritical() << errorText;
            throw std::runtime_error{errorText.toStdString()};
        }

        return result;
    }

    template<class T>
    QSqlQuery Repository<T>::prepare(const QString &queryStr) const
    {
        QSqlQuery query{getDatabase()};
        if (!query.prepare(queryStr))
        {
            const auto error = query.lastError().text();

            qCritical() << error;
            throw std::runtime_error{error.toStdString()};
        }

        return query;
    }

    template<class T>
    void Repository<T>::store(T &entity) const
    {
        preStore(entity);

        if (entity.isNew())
            insert(entity);
        else
            update(entity);

        entity.updateOriginalId();
        entity.setNew(false);

        postStore(entity);
    }

    template<class T>
    template<class U>
    void Repository<T>::batchStore(const U &entities, bool hasId, bool wrapIntransaction) const
    {
        if (entities.empty())
            return;

        const auto maxRowsPerInsert = getMaxRowsPerInsert();
        const auto totalRows = entities.size();
        const auto batches = totalRows / maxRowsPerInsert;

        auto columns = getColumns();

        QStringList columnBindings;
        for (auto i = 0; i < columns.size(); ++i)
            columnBindings << "?";

        if (!hasId)
        {
            columns.removeOne(getIdColumn());
            columnBindings.removeLast();
        }

        const auto bindingStr = QStringLiteral("(") + columnBindings.join(QStringLiteral(", ")) + QStringLiteral(")");

        const auto baseQueryStr = QStringLiteral("REPLACE INTO %1 (%2) VALUES %3")
            .arg(getTableName())
            .arg(columns.join(QStringLiteral(", ")));

        QStringList batchBindings;
        for (auto i = 0u; i < maxRowsPerInsert; ++i)
            batchBindings << bindingStr;

        const auto batchQueryStr = baseQueryStr.arg(batchBindings.join(", "));

        auto db = getDatabase();

        if (wrapIntransaction)
            db.transaction();

        try
        {
            for (auto batch = 0u; batch < batches; ++batch)
            {
                auto query = prepare(batchQueryStr);

                const auto end = std::next(std::begin(entities), (batch + 1) * maxRowsPerInsert);
                for (auto row = std::next(std::begin(entities), batch * maxRowsPerInsert); row != end; ++row)
                    bindPositionalValues(*row, query);

                DatabaseUtils::execQuery(query);
            }

            const auto reminder = totalRows % maxRowsPerInsert;
            if (reminder > 0)
            {
                QStringList restBindings;
                for (auto i = 0u; i < reminder; ++i)
                    restBindings << bindingStr;

                const auto restQueryStr = baseQueryStr.arg(restBindings.join(", "));
                auto query = prepare(restQueryStr);

                for (auto row = std::next(std::begin(entities), batches * maxRowsPerInsert); row != std::end(entities); ++row)
                    bindPositionalValues(*row, query);

                DatabaseUtils::execQuery(query);
            }
        }
        catch (...)
        {
            if (wrapIntransaction)
                db.rollback();

            throw;
        }

        if (wrapIntransaction)
            db.commit();
    }

    template<class T>
    template<class Id>
    void Repository<T>::remove(Id &&id) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE %2 = :id").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(QStringLiteral(":id"), id);
        DatabaseUtils::execQuery(query);
    }

    template<class T>
    typename Repository<T>::EntityList Repository<T>::fetchAll() const
    {
        EntityList out;

        auto result = exec(QStringLiteral("SELECT * FROM %1").arg(getTableName()));
        const auto size = result.size();
        if (size != -1)
            out.reserve(size);

        while (result.next())
            out.emplace_back(populate(result.record()));

        return out;
    }

    template<class T>
    template<class Id>
    typename Repository<T>::EntityPtr Repository<T>::find(Id &&id) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE %2 = :id").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(QStringLiteral(":id"), id);
        DatabaseUtils::execQuery(query);

        if (!query.next())
            throw NotFoundException{};

        return populate(query.record());
    }

    template<class T>
    QSqlDatabase Repository<T>::getDatabase() const
    {
        return mConnectionProvider.getConnection();
    }

    template<class T>
    void Repository<T>::insert(T &entity) const
    {
        auto columns = getColumns();

        QStringList prefixedColumns;
        for (const auto &column : columns)
            prefixedColumns << QStringLiteral(":") + column;

        const auto setNewId = entity.getId() == T::invalidId;
        if (setNewId)
        {
            columns.removeOne(getIdColumn());
            prefixedColumns.removeOne(":" + getIdColumn());
        }

        const auto queryStr = QStringLiteral("REPLACE INTO %1 (%2) VALUES (%3)")
            .arg(getTableName())
            .arg(columns.join(QStringLiteral(", ")))
            .arg(prefixedColumns.join(QStringLiteral(", ")));

        auto query = prepare(queryStr);
        bindValues(entity, query);
        DatabaseUtils::execQuery(query);

        if (setNewId)
        {
            const auto rowId = query.lastInsertId();
            if (!rowId.isNull())
            {
                auto query = prepare(QStringLiteral("SELECT %1 FROM %2 WHERE ROWID = :id").arg(getIdColumn()).arg(getTableName()));
                query.bindValue(QStringLiteral(":id"), rowId);
                DatabaseUtils::execQuery(query);
                query.next();

                entity.setId(query.value(0).template value<typename T::IdType>());
            }
        }
    }

    template<class T>
    void Repository<T>::update(const T &entity) const
    {
        const auto columns = getColumns();

        QStringList updateList;
        for (const auto &column : columns)
            updateList << QStringLiteral("%1 = :%1").arg(column);

        const auto queryStr = QStringLiteral("UPDATE %1 SET %2 WHERE %3 = :id_for_update")
            .arg(getTableName())
            .arg(updateList.join(", "))
            .arg(getIdColumn());

        auto query = prepare(queryStr);
        query.bindValue(QStringLiteral(":id_for_update"), entity.getOriginalId());

        bindValues(entity, query);
        DatabaseUtils::execQuery(query);
    }

    template<class T>
    void Repository<T>::preStore(T &entity) const
    {
        Q_UNUSED(entity);
    }

    template<class T>
    void Repository<T>::postStore(T &entity) const
    {
        Q_UNUSED(entity);
    }

    template<class T>
    size_t Repository<T>::getMaxRowsPerInsert() const
    {
        return maxSqliteBoundVariables / getColumns().count();
    }
}
