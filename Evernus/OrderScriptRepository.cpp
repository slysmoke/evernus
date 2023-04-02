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
#include <QSqlRecord>
#include <QSqlQuery>

#include "OrderScriptRepository.h"

namespace Evernus
{
    QString OrderScriptRepository::getTableName() const
    {
        return "order_scripts";
    }

    QString OrderScriptRepository::getIdColumn() const
    {
        return "id";
    }

    OrderScriptRepository::EntityPtr OrderScriptRepository::populate(const QSqlRecord &record) const
    {
        auto orderScript = std::make_shared<OrderScript>(record.value("id").value<OrderScript::IdType>());
        orderScript->setCode(record.value("code").toString());
        orderScript->setNew(false);

        return orderScript;
    }

    void OrderScriptRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id TEXT PRIMARY KEY,"
            "code TEXT NOT NULL"
        ")"}.arg(getTableName()));
    }

    QStringList OrderScriptRepository::getAllNames() const
    {
        auto query = exec(QString{"SELECT %1 FROM %2 ORDER BY %1"}.arg(getIdColumn()).arg(getTableName()));

        QStringList result;
        while (query.next())
            result << query.value(0).toString();

        return result;
    }

    QStringList OrderScriptRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "code";
    }

    void OrderScriptRepository::bindValues(const OrderScript &entity, QSqlQuery &query) const
    {
        if (entity.getId() != OrderScript::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":code", entity.getCode());
    }

    void OrderScriptRepository::bindPositionalValues(const OrderScript &entity, QSqlQuery &query) const
    {
        if (entity.getId() != OrderScript::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCode());
    }
}
