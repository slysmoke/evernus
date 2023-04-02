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

#include "FilterTextRepository.h"

namespace Evernus
{
    QString FilterTextRepository::getTableName() const
    {
        return "filter_texts";
    }

    QString FilterTextRepository::getIdColumn() const
    {
        return "id";
    }

    FilterTextRepository::EntityPtr FilterTextRepository::populate(const QSqlRecord &record) const
    {
        auto filterText = std::make_shared<FilterText>(record.value("id").value<FilterText::IdType>());
        filterText->setText(record.value("text").toString());
        filterText->setNew(false);

        return filterText;
    }

    void FilterTextRepository::store(const QString &text) const
    {
        auto query = prepare(QString{"REPLACE INTO %1 (text) VALUES (?)"}.arg(getTableName()));
        query.bindValue(0, text);

        DatabaseUtils::execQuery(query);

        emit filtersChanged();
    }

    void FilterTextRepository::remove(const QString &text) const
    {
        remove(std::move(text));
    }

    void FilterTextRepository::remove(QString &&text) const
    {
        auto query = prepare(QString{"DELETE FROM %1 WHERE text = ?"}.arg(getTableName()));
        query.bindValue(0, text);

        DatabaseUtils::execQuery(query);

        emit filtersChanged();
    }

    void FilterTextRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "text TEXT NOT NULL UNIQUE"
        ")"}.arg(getTableName()));
    }

    QStringList FilterTextRepository::fetchRecentlyUsed() const
    {
        auto query = exec(QString{"SELECT text FROM %1 ORDER BY id DESC"}.arg(getTableName()));

        QStringList result;
        while (query.next())
            result << query.value(0).toString();

        return result;
    }

    QStringList FilterTextRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "text";
    }

    void FilterTextRepository::bindValues(const FilterText &entity, QSqlQuery &query) const
    {
        if (entity.getId() != FilterText::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":text", entity.getText());
    }

    void FilterTextRepository::bindPositionalValues(const FilterText &entity, QSqlQuery &query) const
    {
        if (entity.getId() != FilterText::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getText());
    }

    void FilterTextRepository::postStore(FilterText &entity) const
    {
        emit filtersChanged();
    }
}
