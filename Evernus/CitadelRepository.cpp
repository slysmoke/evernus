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
#include <unordered_map>
#include <algorithm>

#include "CitadelRepository.h"

namespace Evernus
{
    QString CitadelRepository::getTableName() const
    {
        return QStringLiteral("citadels");
    }

    QString CitadelRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    CitadelRepository::EntityPtr CitadelRepository::populate(const QSqlRecord &record) const
    {
        auto firstSeen = record.value(QStringLiteral("first_seen")).toDateTime();
        firstSeen.setTimeSpec(Qt::UTC);

        auto lastSeen = record.value(QStringLiteral("last_seen")).toDateTime();
        lastSeen.setTimeSpec(Qt::UTC);

        auto citadel = std::make_shared<Citadel>(record.value(QStringLiteral("id")).value<Citadel::IdType>());
        citadel->setName(record.value(QStringLiteral("name")).toString());
        citadel->setSolarSystemId(record.value(QStringLiteral("solar_system_id")).toULongLong());
        citadel->setRegionId(record.value(QStringLiteral("region_id")).toUInt());
        citadel->setTypeId(record.value(QStringLiteral("type_id")).toUInt());
        citadel->setX(record.value(QStringLiteral("x")).toDouble());
        citadel->setY(record.value(QStringLiteral("y")).toDouble());
        citadel->setZ(record.value(QStringLiteral("z")).toDouble());
        citadel->setFirstSeen(firstSeen);
        citadel->setLastSeen(lastSeen);
        citadel->setPublic(record.value(QStringLiteral("public")).toBool());
        citadel->setIgnored(record.value(QStringLiteral("ignored")).toBool());
        citadel->setNew(false);

        return citadel;
    }

    void CitadelRepository::create() const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "name TEXT NOT NULL,"
            "solar_system_id INTEGER NOT NULL,"
            "region_id INTEGER NOT NULL,"
            "x REAL NOT NULL,"
            "y REAL NOT NULL,"
            "z REAL NOT NULL,"
            "last_seen DATETIME NOT NULL,"
            "first_seen DATETIME NOT NULL,"
            "type_id INTEGER NOT NULL,"
            "public INTEGER NOT NULL,"
            "ignored INTEGER NOT NULL"
        ")").arg(getTableName()));

        try
        {
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_region ON %1(region_id)").arg(getTableName()));
        }
        catch (const std::runtime_error &)
        {
            // ignore - versions < 1.49 do not have this column
            qDebug() << "SQL errors ignored";
        }
    }

    void CitadelRepository::deleteAll() const
    {
        exec(QStringLiteral("DELETE FROM %1").arg(getTableName()));
    }

    void CitadelRepository::replace(CitadelList citadels, bool deleteExisting) const
    {
        auto start = std::begin(citadels);
        const auto end = std::end(citadels);

        std::unordered_map<Citadel::IdType, bool> stateMap;
        stateMap.reserve(citadels.size());

        while (start != end)
        {
            const auto size = std::min(maxSqliteBoundVariables, static_cast<std::size_t>(std::distance(start, end)));

            QStringList placeholders;
            std::fill_n(std::back_inserter(placeholders), size, QStringLiteral("?"));

            const auto fill = placeholders.join(QStringLiteral(", "));

            auto query = prepare(QStringLiteral("SELECT %3, ignored FROM %1 WHERE id IN (%2)")
                .arg(getTableName())
                .arg(fill)
                .arg(getIdColumn()));

            for (auto i = 0u; i < size; ++i)
                query.addBindValue((start++)->getId());

            DatabaseUtils::execQuery(query);

            while (query.next())
                stateMap.emplace(query.value(0).value<Citadel::IdType>(), query.value(1).toBool());
        }

        for (auto &citadel : citadels)
            citadel.setIgnored(stateMap[citadel.getId()]);

        auto db = getDatabase();
        db.transaction();

        try
        {
            if (deleteExisting)
                deleteAll();

            batchStore(citadels, true, false);
        }
        catch (...)
        {
            db.rollback();
            throw;
        }

        db.commit();
    }

    CitadelRepository::EntityList CitadelRepository::fetchForSolarSystem(uint solarSystemId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE solar_system_id = ?").arg(getTableName()));
        query.bindValue(0, solarSystemId);

        return buildList(query);
    }

    CitadelRepository::EntityList CitadelRepository::fetchForRegion(uint regionId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE region_id = ?").arg(getTableName()));
        query.bindValue(0, regionId);

        return buildList(query);
    }

    void CitadelRepository::setIgnored(const CitadelIdList &citadels) const
    {
        auto db = getDatabase();
        db.transaction();

        try
        {
            exec(QStringLiteral("UPDATE %1 SET ignored = 0").arg(getTableName()));

            auto start = std::begin(citadels);
            const auto end = std::end(citadels);

            while (start != end)
            {
                const auto size = std::min(maxSqliteBoundVariables, static_cast<std::size_t>(std::distance(start, end)));

                QStringList placeholders;
                std::fill_n(std::back_inserter(placeholders), size, QStringLiteral("?"));

                const auto fill = placeholders.join(QStringLiteral(", "));

                auto query = prepare(QStringLiteral("UPDATE %1 SET ignored = 1 WHERE id IN (%2)").arg(getTableName()).arg(fill));

                for (auto i = 0u; i < size; ++i)
                    query.addBindValue(*start++);

                DatabaseUtils::execQuery(query);
            }
        }
        catch (...)
        {
            db.rollback();
            throw;
        }

        db.commit();
    }

    QStringList CitadelRepository::getColumns() const
    {
        return QStringList{}
            << QStringLiteral("id")
            << QStringLiteral("name")
            << QStringLiteral("solar_system_id")
            << QStringLiteral("region_id")
            << QStringLiteral("x")
            << QStringLiteral("y")
            << QStringLiteral("z")
            << QStringLiteral("last_seen")
            << QStringLiteral("first_seen")
            << QStringLiteral("type_id")
            << QStringLiteral("public")
            << QStringLiteral("ignored");
    }

    void CitadelRepository::bindValues(const Citadel &entity, QSqlQuery &query) const
    {
        if (entity.getId() != Citadel::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":name"), entity.getName());
        query.bindValue(QStringLiteral(":solar_system_id"), entity.getSolarSystemId());
        query.bindValue(QStringLiteral(":region_id"), entity.getRegionId());
        query.bindValue(QStringLiteral(":x"), entity.getX());
        query.bindValue(QStringLiteral(":y"), entity.getY());
        query.bindValue(QStringLiteral(":z"), entity.getZ());
        query.bindValue(QStringLiteral(":last_seen"), entity.getLastSeen());
        query.bindValue(QStringLiteral(":first_seen"), entity.getFirstSeen());
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
        query.bindValue(QStringLiteral(":public"), entity.isPublic());
        query.bindValue(QStringLiteral(":ignored"), entity.isIgnored());
    }

    void CitadelRepository::bindPositionalValues(const Citadel &entity, QSqlQuery &query) const
    {
        if (entity.getId() != Citadel::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getName());
        query.addBindValue(entity.getSolarSystemId());
        query.addBindValue(entity.getRegionId());
        query.addBindValue(entity.getX());
        query.addBindValue(entity.getY());
        query.addBindValue(entity.getZ());
        query.addBindValue(entity.getLastSeen());
        query.addBindValue(entity.getFirstSeen());
        query.addBindValue(entity.getTypeId());
        query.addBindValue(entity.isPublic());
        query.addBindValue(entity.isIgnored());
    }

    CitadelRepository::EntityList CitadelRepository::buildList(QSqlQuery &query) const
    {
        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }
}
