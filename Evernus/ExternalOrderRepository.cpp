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

#include "MarketOrder.h"
#include "Citadel.h"

#include "ExternalOrderRepository.h"

namespace Evernus
{
    QString ExternalOrderRepository::getTableName() const
    {
        return QStringLiteral("external_orders");
    }

    QString ExternalOrderRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    ExternalOrderRepository::EntityPtr ExternalOrderRepository::populate(const QSqlRecord &record) const
    {
        auto updateDt = record.value(QStringLiteral("update_time")).toDateTime();
        updateDt.setTimeSpec(Qt::UTC);

        auto issuedDt = record.value(QStringLiteral("issued")).toDateTime();
        issuedDt.setTimeSpec(Qt::UTC);

        auto externalOrder = std::make_shared<ExternalOrder>(record.value(QStringLiteral("id")).value<ExternalOrder::IdType>());
        externalOrder->setType(static_cast<ExternalOrder::Type>(record.value(QStringLiteral("type")).toInt()));
        externalOrder->setTypeId(record.value(QStringLiteral("type_id")).value<ExternalOrder::TypeIdType>());
        externalOrder->setStationId(record.value(QStringLiteral("location_id")).toULongLong());
        externalOrder->setSolarSystemId(record.value(QStringLiteral("solar_system_id")).toUInt());
        externalOrder->setRegionId(record.value(QStringLiteral("region_id")).toUInt());
        externalOrder->setRange(record.value(QStringLiteral("range")).toInt());
        externalOrder->setUpdateTime(updateDt);
        externalOrder->setPrice(record.value(QStringLiteral("value")).toDouble());
        externalOrder->setVolumeEntered(record.value(QStringLiteral("volume_entered")).toUInt());
        externalOrder->setVolumeRemaining(record.value(QStringLiteral("volume_remaining")).toUInt());
        externalOrder->setMinVolume(record.value(QStringLiteral("min_volume")).toUInt());
        externalOrder->setIssued(issuedDt);
        externalOrder->setDuration(record.value(QStringLiteral("duration")).value<short>());
        externalOrder->setNew(false);

        return externalOrder;
    }

    void ExternalOrderRepository::create() const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id BIGINT PRIMARY KEY,"
            "type TINYINT NOT NULL,"
            "type_id INTEGER NOT NULL,"
            "location_id BIGINT NOT NULL,"
            "solar_system_id INTEGER NOT NULL,"
            "region_id INTEGER NOT NULL,"
            "range INTEGER NOT NULL,"
            "update_time DATETIME NOT NULL,"
            "value DOUBLE NOT NULL,"
            "volume_entered INTEGER NOT NULL,"
            "volume_remaining INTEGER NOT NULL,"
            "min_volume INTEGER NOT NULL,"
            "issued DATETIME NOT NULL,"
            "duration INTEGER NOT NULL"
        ")").arg(getTableName()));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_id ON %1(type_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_id_region ON %1(type_id, region_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_solar_system ON %1(solar_system_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_region ON %1(region_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_update_time ON %1(update_time)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_type_id ON %1(type, type_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_type_id_location ON %1(type, type_id, location_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_type_id_solar_system ON %1(type, type_id, solar_system_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_type_id_region ON %1(type, type_id, region_id)").arg(getTableName()));
    }

    ExternalOrderRepository::EntityPtr ExternalOrderRepository::findSellByTypeAndStation(ExternalOrder::TypeIdType typeId,
                                                                                         quint64 stationId,
                                                                                         const Repository<MarketOrder> &orderRepo,
                                                                                         const Repository<MarketOrder> &corpOrderRepo) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT * FROM %1 WHERE type = ? AND type_id = ? AND location_id = ? AND id NOT IN "
            "(SELECT id FROM %2 WHERE state = ? UNION SELECT id FROM %3 WHERE state = ?) "
            "ORDER BY value ASC LIMIT 1")
            .arg(getTableName()).arg(orderRepo.getTableName()).arg(corpOrderRepo.getTableName()));
        query.addBindValue(static_cast<int>(ExternalOrder::Type::Sell));
        query.addBindValue(typeId);
        query.addBindValue(stationId);
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);
        if (!query.next())
            BOOST_THROW_EXCEPTION(NotFoundException{});

        return populate(query.record());
    }

    ExternalOrderRepository::EntityPtr ExternalOrderRepository::findSellByTypeAndRegion(ExternalOrder::TypeIdType typeId,
                                                                                        uint regionId,
                                                                                        const Repository<MarketOrder> &orderRepo,
                                                                                        const Repository<MarketOrder> &corpOrderRepo) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT * FROM %1 WHERE type = ? AND type_id = ? AND region_id = ? AND id NOT IN "
            "(SELECT id FROM %2 WHERE state = ? UNION SELECT id FROM %3 WHERE state = ?) "
            "ORDER BY value ASC LIMIT 1")
            .arg(getTableName()).arg(orderRepo.getTableName()).arg(corpOrderRepo.getTableName()));
        query.addBindValue(static_cast<int>(ExternalOrder::Type::Sell));
        query.addBindValue(typeId);
        query.addBindValue(regionId);
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);
        if (!query.next())
            BOOST_THROW_EXCEPTION(NotFoundException{});

        return populate(query.record());
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::findBuyByTypeAndRegion(ExternalOrder::TypeIdType typeId,
                                                                                        uint regionId,
                                                                                        const Repository<MarketOrder> &orderRepo,
                                                                                        const Repository<MarketOrder> &corpOrderRepo) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT * FROM %1 WHERE type = ? AND type_id = ? AND region_id = ? AND id NOT IN "
            "(SELECT id FROM %2 WHERE state = ? UNION SELECT id FROM %3 WHERE state = ?)"
            ).arg(getTableName()).arg(orderRepo.getTableName()).arg(corpOrderRepo.getTableName()));
        query.addBindValue(static_cast<int>(ExternalOrder::Type::Buy));
        query.addBindValue(typeId);
        query.addBindValue(regionId);
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));
        query.addBindValue(static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchBuyByType(ExternalOrder::TypeIdType typeId) const
    {
        return fetchByType(typeId, ExternalOrder::Type::Buy);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchBuyByTypeAndStation(ExternalOrder::TypeIdType typeId,
                                                                                          quint64 stationId) const
    {
        return fetchByTypeAndStation(typeId, stationId, ExternalOrder::Type::Buy);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchBuyByTypeAndSolarSystem(ExternalOrder::TypeIdType typeId,
                                                                                              uint solarSystemId) const
    {
        return fetchByTypeAndSolarSystem(typeId, solarSystemId, ExternalOrder::Type::Buy);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchBuyByTypeAndRegion(ExternalOrder::TypeIdType typeId,
                                                                                         uint regionId) const
    {
        return fetchByTypeAndRegion(typeId, regionId, ExternalOrder::Type::Buy);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchSellByType(ExternalOrder::TypeIdType typeId) const
    {
        return fetchByType(typeId, ExternalOrder::Type::Sell);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchSellByTypeAndStation(ExternalOrder::TypeIdType typeId,
                                                                                           quint64 stationId) const
    {
        return fetchByTypeAndStation(typeId, stationId, ExternalOrder::Type::Sell);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchSellByTypeAndSolarSystem(ExternalOrder::TypeIdType typeId,
                                                                                               uint solarSystemId) const
    {
        return fetchByTypeAndSolarSystem(typeId, solarSystemId, ExternalOrder::Type::Sell);
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchSellByTypeAndRegion(ExternalOrder::TypeIdType typeId,
                                                                                          uint regionId) const
    {
        return fetchByTypeAndRegion(typeId, regionId, ExternalOrder::Type::Sell);
    }

    std::vector<EveType::IdType> ExternalOrderRepository::fetchUniqueTypes() const
    {
        return fetchUniqueColumn<EveType::IdType>(QStringLiteral("type_id"));
    }

    std::vector<uint> ExternalOrderRepository::fetchUniqueRegions() const
    {
        return fetchUniqueColumn<uint>(QStringLiteral("region_id"));
    }

    std::vector<uint> ExternalOrderRepository::fetchUniqueSolarSystems(uint regionId) const
    {
        if (regionId == 0)
            return fetchUniqueColumn<uint>(QStringLiteral("solar_system_id"));

        std::vector<uint> result;

        auto query = prepare(QStringLiteral("SELECT DISTINCT solar_system_id FROM %1 WHERE region_id = ?").arg(getTableName()));
        query.bindValue(0, regionId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(query.value(0).toUInt());

        return result;
    }

    std::vector<quint64> ExternalOrderRepository::fetchUniqueStations() const
    {
        return fetchUniqueColumn<quint64>(QStringLiteral("location_id"));
    }

    std::vector<quint64> ExternalOrderRepository::fetchUniqueStationsByRegion(uint regionId) const
    {
        std::vector<quint64> result;

        auto query = prepare(QStringLiteral("SELECT DISTINCT location_id FROM %1 WHERE region_id = ?").arg(getTableName()));
        query.bindValue(0, regionId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(query.value(0).toULongLong());

        return result;
    }

    std::vector<quint64> ExternalOrderRepository::fetchUniqueStationsBySolarSystem(uint solarSystemId) const
    {
        std::vector<quint64> result;

        auto query = prepare(QStringLiteral("SELECT DISTINCT location_id FROM %1 WHERE solar_system_id = ?").arg(getTableName()));
        query.bindValue(0, solarSystemId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(query.value(0).toULongLong());

        return result;
    }

    void ExternalOrderRepository::removeObsolete(const TypeLocationPairs &set) const
    {
        if (set.empty())
            return;

        auto db = getDatabase();

        db.transaction();

        try
        {
            const auto baseQuery = QStringLiteral("DELETE FROM %1 WHERE %2").arg(getTableName());
            const auto baseWhere = QStringLiteral("(type_id = ? AND region_id = ?)");

            const auto batchSize = 300;
            const auto batches = set.size() / batchSize;

            auto it = std::begin(set);

            QStringList batchWhere;
            for (auto i = 0; i < batchSize; ++i)
                batchWhere << baseWhere;

            const auto batchQuery = baseQuery.arg(batchWhere.join(QStringLiteral(" OR ")));

            for (auto i = 0u; i < batches; ++i)
            {
                auto query = prepare(batchQuery);

                for (auto j = 0; j < batchSize; ++j)
                {
                    query.addBindValue(it->first);
                    query.addBindValue(it->second);

                    ++it;
                }

                DatabaseUtils::execQuery(query);
            }

            const auto reminder = set.size() % batchSize;
            if (reminder == 0)
                return;

            QStringList reminderWhere;
            for (auto i = 0u; i < reminder; ++i)
                reminderWhere << baseWhere;

            const auto reminderQuery = baseQuery.arg(reminderWhere.join(QStringLiteral(" OR ")));
            auto query = prepare(reminderQuery);

            for (auto i = 0u; i < reminder; ++i)
            {
                query.addBindValue(it->first);
                query.addBindValue(it->second);

                ++it;
            }

            DatabaseUtils::execQuery(query);
        }
        catch (...)
        {
            db.rollback();
            throw;
        }

        db.commit();
    }

    void ExternalOrderRepository::removeForType(ExternalOrder::TypeIdType typeId) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE type_id = ?").arg(getTableName()));
        query.bindValue(0, typeId);

        DatabaseUtils::execQuery(query);
    }

    void ExternalOrderRepository::removeAll() const
    {
        exec(QStringLiteral("DELETE FROM %1").arg(getTableName()));
    }

    void ExternalOrderRepository::fixMissingData(const Repository<Citadel> &citadelRepo) const
    {
        exec(QStringLiteral(R"(
    UPDATE %1 SET solar_system_id = (
        SELECT c.solar_system_id FROM %2 c WHERE c.%3 = location_id
    ) WHERE solar_system_id = 0 AND EXISTS(
        SELECT c.solar_system_id FROM %2 c WHERE c.%3 = location_id
    )
        )").arg(getTableName()).arg(citadelRepo.getTableName()).arg(citadelRepo.getIdColumn()));
    }

    QStringList ExternalOrderRepository::getColumns() const
    {
        return {
            QStringLiteral("id"),
            QStringLiteral("type"),
            QStringLiteral("type_id"),
            QStringLiteral("location_id"),
            QStringLiteral("solar_system_id"),
            QStringLiteral("region_id"),
            QStringLiteral("range"),
            QStringLiteral("update_time"),
            QStringLiteral("value"),
            QStringLiteral("volume_entered"),
            QStringLiteral("volume_remaining"),
            QStringLiteral("min_volume"),
            QStringLiteral("issued"),
            QStringLiteral("duration")
        };
    }

    void ExternalOrderRepository::bindValues(const ExternalOrder &entity, QSqlQuery &query) const
    {
        if (entity.getId() != ExternalOrder::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":type"), static_cast<int>(entity.getType()));
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
        query.bindValue(QStringLiteral(":location_id"), entity.getStationId());
        query.bindValue(QStringLiteral(":solar_system_id"), entity.getSolarSystemId());
        query.bindValue(QStringLiteral(":region_id"), entity.getRegionId());
        query.bindValue(QStringLiteral(":range"), entity.getRange());
        query.bindValue(QStringLiteral(":update_time"), entity.getUpdateTime());
        query.bindValue(QStringLiteral(":value"), entity.getPrice());
        query.bindValue(QStringLiteral(":volume_entered"), entity.getVolumeEntered());
        query.bindValue(QStringLiteral(":volume_remaining"), entity.getVolumeRemaining());
        query.bindValue(QStringLiteral(":min_volume"), entity.getMinVolume());
        query.bindValue(QStringLiteral(":issued"), entity.getIssued());
        query.bindValue(QStringLiteral(":duration"), entity.getDuration());
    }

    void ExternalOrderRepository::bindPositionalValues(const ExternalOrder &entity, QSqlQuery &query) const
    {
        if (entity.getId() != ExternalOrder::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(static_cast<int>(entity.getType()));
        query.addBindValue(entity.getTypeId());
        query.addBindValue(entity.getStationId());
        query.addBindValue(entity.getSolarSystemId());
        query.addBindValue(entity.getRegionId());
        query.addBindValue(entity.getRange());
        query.addBindValue(entity.getUpdateTime());
        query.addBindValue(entity.getPrice());
        query.addBindValue(entity.getVolumeEntered());
        query.addBindValue(entity.getVolumeRemaining());
        query.addBindValue(entity.getMinVolume());
        query.addBindValue(entity.getIssued());
        query.addBindValue(entity.getDuration());
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchByType(ExternalOrder::TypeIdType typeId,
                                                                             ExternalOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type = ? AND type_id = ?").arg(getTableName()));
        query.addBindValue(static_cast<int>(type));
        query.addBindValue(typeId);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchByTypeAndStation(ExternalOrder::TypeIdType typeId,
                                                                                       quint64 stationId,
                                                                                       ExternalOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type = ? AND type_id = ? AND location_id = ?").arg(getTableName()));
        query.addBindValue(static_cast<int>(type));
        query.addBindValue(typeId);
        query.addBindValue(stationId);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchByTypeAndSolarSystem(ExternalOrder::TypeIdType typeId,
                                                                                           uint solarSystemId,
                                                                                           ExternalOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type = ? AND type_id = ? AND solar_system_id = ?").arg(getTableName()));
        query.addBindValue(static_cast<int>(type));
        query.addBindValue(typeId);
        query.addBindValue(solarSystemId);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    ExternalOrderRepository::EntityList ExternalOrderRepository::fetchByTypeAndRegion(ExternalOrder::TypeIdType typeId,
                                                                                      uint regionId,
                                                                                      ExternalOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type = ? AND type_id = ? AND region_id = ?").arg(getTableName()));
        query.addBindValue(static_cast<int>(type));
        query.addBindValue(typeId);
        query.addBindValue(regionId);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    template<class T>
    std::vector<T> ExternalOrderRepository::fetchUniqueColumn(const QString &column) const
    {
        std::vector<T> result;

        auto query = exec(QStringLiteral("SELECT DISTINCT %1 FROM %2").arg(column).arg(getTableName()));

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(query.value(0).template value<T>());

        return result;
    }
}
