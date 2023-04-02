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
#include <boost/throw_exception.hpp>

#include <QSqlRecord>
#include <QSqlQuery>

#include "MetaGroupRepository.h"

namespace Evernus
{
    QString MetaGroupRepository::getTableName() const
    {
        return "invMetaGroups";
    }

    QString MetaGroupRepository::getIdColumn() const
    {
        return "metaGroupID";
    }

    MetaGroupRepository::EntityPtr MetaGroupRepository::populate(const QSqlRecord &record) const
    {
        const auto description = record.value("description");

        auto group = std::make_shared<MetaGroup>(record.value("metaGroupID").value<MetaGroup::IdType>());
        group->setName(record.value("metaGroupName").toString());
        group->setDescription((description.isNull()) ? (MetaGroup::DescriptionType{}) : (description.toString()));
        group->setNew(false);

        return group;
    }

    MetaGroupRepository::EntityPtr MetaGroupRepository::fetchForType(EveType::IdType id) const
    {
        auto query = prepare(QString{"SELECT * FROM %1 WHERE %2 = (SELECT metaGroupID FROM invMetaTypes WHERE typeID = ?)"}
            .arg(getTableName())
            .arg(getIdColumn()));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);
        if (!query.next())
            BOOST_THROW_EXCEPTION(NotFoundException{});

        return populate(query.record());
    }

    QStringList MetaGroupRepository::getColumns() const
    {
        return QStringList{}
            << "metaGroupID"
            << "metaGroupName"
            << "description";
    }

    void MetaGroupRepository::bindValues(const MetaGroup &entity, QSqlQuery &query) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(query);

        BOOST_THROW_EXCEPTION(std::logic_error{"Meta group repository is read-only."});
    }

    void MetaGroupRepository::bindPositionalValues(const MetaGroup &entity, QSqlQuery &query) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(query);

        BOOST_THROW_EXCEPTION(std::logic_error{"Meta group repository is read-only."});
    }
}
