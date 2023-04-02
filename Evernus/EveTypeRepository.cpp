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

#include "EveTypeRepository.h"

namespace Evernus
{
    QString EveTypeRepository::getTableName() const
    {
        return QStringLiteral("invTypes");
    }

    QString EveTypeRepository::getIdColumn() const
    {
        return QStringLiteral("typeID");
    }

    EveTypeRepository::EntityPtr EveTypeRepository::populate(const QSqlRecord &record) const
    {
        const auto marketGroupId = record.value(QStringLiteral("marketGroupID"));
        const auto description = record.value(QStringLiteral("description"));
        const auto raceId = record.value(QStringLiteral("raceID"));

        auto type = std::make_shared<EveType>(record.value(QStringLiteral("typeID")).value<EveType::IdType>());
        type->setGroupId(record.value(QStringLiteral("groupID")).toUInt());
        type->setName(record.value(QStringLiteral("typeName")).toString());
        type->setDescription((description.isNull()) ? (EveType::DescriptionType{}) : (description.toString()));
        type->setMass(record.value(QStringLiteral("mass")).toDouble());
        type->setVolume(record.value(QStringLiteral("volume")).toDouble());
        type->setCapacity(record.value(QStringLiteral("capacity")).toDouble());
        type->setPortionSize(record.value(QStringLiteral("portionSize")).toInt());
        type->setRaceId((raceId.isNull()) ? (EveType::RaceIdType{}) : (raceId.value<EveType::RaceIdType::value_type>()));
        type->setBasePrice(record.value(QStringLiteral("basePrice")).toDouble());
        type->setPublished(record.value(QStringLiteral("published")).toInt() != 0);
        type->setMarketGroupId((marketGroupId.isNull()) ? (EveType::MarketGroupIdType{}) : (marketGroupId.value<MarketGroup::IdType>()));
        type->setGraphicId(record.value(QStringLiteral("graphicID")).toUInt());
        type->setIconId(record.value(QStringLiteral("iconID")).toUInt());
        type->setRadius(record.value(QStringLiteral("radius")).toDouble());
        type->setSoundId(record.value(QStringLiteral("soundID")).toUInt());
        type->setNew(false);

        return type;
    }

    std::unordered_map<EveType::IdType, QString> EveTypeRepository::fetchAllTradeableNames() const
    {
        auto query = exec(QStringLiteral("SELECT %1, typeName FROM %2 WHERE published = 1 AND marketGroupID IS NOT NULL")
            .arg(getIdColumn()).arg(getTableName()));

        std::unordered_map<EveType::IdType, QString> result;
        while (query.next())
            result[query.value(0).toUInt()] = query.value(1).toString();

        return result;
    }

    EveTypeRepository::TypeList EveTypeRepository::fetchAllTradeableIds() const
    {
        auto query = exec(QStringLiteral("SELECT %1 FROM %2 WHERE published = 1 AND marketGroupID IS NOT NULL")
            .arg(getIdColumn()).arg(getTableName()));

        TypeList result;
        while (query.next())
            result.emplace(query.value(0).toUInt());

        return result;
    }

    EveTypeRepository::TypeList EveTypeRepository::fetchCitadelIds() const
    {
        auto query = exec(QStringLiteral("SELECT %1 FROM %2 WHERE groupID = 1657")  // hardcoded citadel group, because why not
            .arg(getIdColumn()).arg(getTableName()));

        TypeList result;
        while (query.next())
            result.emplace(query.value(0).toUInt());

        return result;
    }

    EveTypeRepository::EntityList EveTypeRepository::fetchAllTradeable() const
    {
        EntityList out;

        auto result = exec(QStringLiteral("SELECT * FROM %1 WHERE published = 1 AND marketGroupID IS NOT NULL").arg(getTableName()));
        const auto size = result.size();
        if (size != -1)
            out.reserve(size);

        while (result.next())
            out.emplace_back(populate(result.record()));

        return out;
    }

    QStringList EveTypeRepository::getColumns() const
    {
        return QStringList{}
            << QStringLiteral("typeID")
            << QStringLiteral("groupID")
            << QStringLiteral("typeName")
            << QStringLiteral("description")
            << QStringLiteral("mass")
            << QStringLiteral("volume")
            << QStringLiteral("capacity")
            << QStringLiteral("portionSize")
            << QStringLiteral("raceID")
            << QStringLiteral("basePrice")
            << QStringLiteral("published")
            << QStringLiteral("marketGroupID")
            << QStringLiteral("graphicID")
            << QStringLiteral("iconID")
            << QStringLiteral("radius")
            << QStringLiteral("soundID");
    }

    void EveTypeRepository::bindValues(const EveType &entity, QSqlQuery &query) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(query);

        BOOST_THROW_EXCEPTION(std::logic_error{"Type repository is read-only."});
    }

    void EveTypeRepository::bindPositionalValues(const EveType &entity, QSqlQuery &query) const
    {
        Q_UNUSED(entity);
        Q_UNUSED(query);

        BOOST_THROW_EXCEPTION(std::logic_error{"Type repository is read-only."});
    }
}
