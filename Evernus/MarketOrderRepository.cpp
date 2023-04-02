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
#include <QTextStream>
#include <QSqlRecord>
#include <QSqlQuery>

#include "MarketOrderRepository.h"

namespace Evernus
{
    MarketOrderRepository::MarketOrderRepository(bool corp, const DatabaseConnectionProvider &connectionProvider)
        : Repository{connectionProvider}
        , mCorp{corp}
    {
    }

    QString MarketOrderRepository::getTableName() const
    {
        return (mCorp) ? (QStringLiteral("corp_market_orders")) : (QStringLiteral("market_orders"));
    }

    QString MarketOrderRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    MarketOrderRepository::EntityPtr MarketOrderRepository::populate(const QSqlRecord &record) const
    {
        auto issued = record.value(QStringLiteral("issued")).toDateTime();
        issued.setTimeSpec(Qt::UTC);

        auto firstSeen = record.value(QStringLiteral("first_seen")).toDateTime();
        firstSeen.setTimeSpec(Qt::UTC);

        auto lastSeen = record.value(QStringLiteral("last_seen")).toDateTime();
        lastSeen.setTimeSpec(Qt::UTC);

        auto marketOrder = std::make_shared<MarketOrder>(record.value(getIdColumn()).value<MarketOrder::IdType>());
        marketOrder->setCharacterId(record.value(QStringLiteral("character_id")).value<Character::IdType>());
        marketOrder->setStationId(record.value(QStringLiteral("location_id")).toULongLong());
        marketOrder->setVolumeEntered(record.value(QStringLiteral("volume_entered")).toUInt());
        marketOrder->setVolumeRemaining(record.value(QStringLiteral("volume_remaining")).toUInt());
        marketOrder->setMinVolume(record.value(QStringLiteral("min_volume")).toUInt());
        marketOrder->setDelta(record.value(QStringLiteral("delta")).toInt());
        marketOrder->setState(static_cast<MarketOrder::State>(record.value(QStringLiteral("state")).toInt()));
        marketOrder->setTypeId(record.value(QStringLiteral("type_id")).value<EveType::IdType>());
        marketOrder->setRange(record.value(QStringLiteral("range")).value<short>());
        marketOrder->setAccountKey(record.value(QStringLiteral("account_key")).value<short>());
        marketOrder->setDuration(record.value(QStringLiteral("duration")).value<short>());
        marketOrder->setEscrow(record.value(QStringLiteral("escrow")).toDouble());
        marketOrder->setPrice(record.value(QStringLiteral("price")).toDouble());
        marketOrder->setType(static_cast<MarketOrder::Type>(record.value(QStringLiteral("type")).toInt()));
        marketOrder->setIssued(issued);
        marketOrder->setFirstSeen(firstSeen);
        marketOrder->setLastSeen(lastSeen);
        marketOrder->setCorporationId(record.value(QStringLiteral("corporation_id")).toULongLong());
        if (!record.value(QStringLiteral("notes")).isNull())
            marketOrder->setNotes(record.value(QStringLiteral("notes")).toString());
        if (!record.value(QStringLiteral("custom_location_id")).isNull())
            marketOrder->setCustomStationId(record.value(QStringLiteral("custom_location_id")).toULongLong());
        if (!record.value(QStringLiteral("color_tag")).isNull())
            marketOrder->setColorTag(record.value(QStringLiteral("color_tag")).toString());
        marketOrder->setNew(false);

        return marketOrder;
    }

    void MarketOrderRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id BIGINT PRIMARY KEY,"
            "character_id BIGINT NOT NULL %2,"
            "location_id INTEGER NOT NULL,"
            "volume_entered INTEGER NOT NULL,"
            "volume_remaining INTEGER NOT NULL,"
            "min_volume INTEGER NOT NULL,"
            "delta INTEGER NOT NULL DEFAULT 0,"
            "state TINYINT NOT NULL,"
            "type_id INTEGER NOT NULL,"
            "range INTEGER NOT NULL,"
            "account_key INTEGER NOT NULL,"
            "duration INTEGER NOT NULL,"
            "escrow NUMERIC NOT NULL,"
            "price NUMERIC NOT NULL,"
            "type TINYINT NOT NULL,"
            "issued DATETIME NOT NULL,"
            "first_seen DATETIME NOT NULL,"
            "last_seen DATETIME NULL,"
            "corporation_id BIGINT NOT NULL,"
            "notes TEXT NULL,"
            "custom_location_id INTEGER NULL,"
            "color_tag TEXT NULL"
        ")").arg(getTableName()).arg(
            (mCorp) ? (QString{}) : (QStringLiteral("REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE").arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()))));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)").arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_state ON %1(state)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_state ON %1(character_id, state)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_type ON %1(character_id, type)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_last_seen ON %1(character_id, last_seen)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_last_seen ON %1(last_seen)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_first_seen_last_seen_state ON %1(first_seen, last_seen, state)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_first_seen_last_seen_state ON %1(character_id, first_seen, last_seen, state)").arg(getTableName()));

        try
        {
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_type ON %1(corporation_id, type)").arg(getTableName()));
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_last_seen ON %1(corporation_id, last_seen)").arg(getTableName()));
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_first_seen_last_seen_state ON %1(corporation_id, first_seen, last_seen, state)").arg(getTableName()));
        }
        catch (const std::runtime_error &)
        {
            // ignore - versions < 0.5 do not have this column
            qDebug() << "SQL errors ignored";
        }
    }

    void MarketOrderRepository::dropIndexes(const Repository<Character> &characterRepo) const
    {
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_%2_index").arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_state").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_corporation_type").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_character_state").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_character_type").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_character_last_seen").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_corporation_last_seen").arg(getTableName()));
        exec(QStringLiteral("DROP INDEX IF EXISTS %1_last_seen").arg(getTableName()));
    }

    void MarketOrderRepository::copyDataWithoutCorporationIdFrom(const QString &table) const
    {
        exec(QStringLiteral("REPLACE INTO %1 "
            "(id, character_id, location_id, volume_entered, volume_remaining, min_volume, delta, state, type_id, range,"
             "account_key, duration, escrow, price, type, issued, first_seen, last_seen, corporation_id, notes, color_tag) "
            "SELECT id, character_id, location_id, volume_entered, volume_remaining, min_volume, delta, state, type_id, range,"
                   "account_key, duration, escrow, price, type, issued, first_seen, last_seen, 0, notes, color_tag "
            "FROM %2").arg(getTableName()).arg(table));
    }

    MarketOrderRepository::AggrData MarketOrderRepository::getAggregatedData(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT type, COUNT(*), SUM(price * volume_remaining), SUM(volume_remaining) FROM %1 WHERE character_id = ? AND state = ? GROUP BY type")
                .arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        AggrData data;

        for (auto i = 0; i < 2; ++i)
        {
            if (query.next())
            {
                SingleAggrData *singleData = nullptr;

                const auto type = static_cast<MarketOrder::Type>(query.value(0).toInt());
                if (type == MarketOrder::Type::Buy)
                    singleData = &data.mBuyData;
                else if (type == MarketOrder::Type::Sell)
                    singleData = &data.mSellData;

                if (singleData != nullptr)
                {
                    singleData->mCount = query.value(1).toUInt();
                    singleData->mPriceSum = query.value(2).toDouble();
                    singleData->mVolume = query.value(3).toUInt();
                }
            }
        }

        return data;
    }

    MarketOrderRepository::CustomAggregatedData MarketOrderRepository::getCustomAggregatedData(Character::IdType characterId,
                                                                                               AggregateColumn groupingColumn,
                                                                                               AggregateOrderColumn orderColumn,
                                                                                               int limit,
                                                                                               bool includeActive,
                                                                                               bool includeNotFulfilled) const
    {
        CustomAggregatedData result;

        QString queryStr{
            "SELECT %1, COUNT(*), SUM(price), SUM(volume_entered) "
            "FROM %2 "
            "WHERE character_id = ? %3 "
            "GROUP BY %1 "
            "ORDER BY %4 DESC "
            "LIMIT %5"};

        switch (groupingColumn) {
        case AggregateColumn::TypeId:
            queryStr = queryStr.arg(QStringLiteral("type_id"));
            break;
        case AggregateColumn::LocationId:
            queryStr = queryStr.arg(QStringLiteral("location_id"));
            break;
        default:
            return result;
        }

        queryStr = queryStr.arg(getTableName());

        if (includeActive && includeNotFulfilled)
            queryStr = queryStr.arg(QString{});
        else if (includeActive)
            queryStr = queryStr.arg(QStringLiteral("AND state IN (?, ?)"));
        else if (includeNotFulfilled)
            queryStr = queryStr.arg(QStringLiteral("AND state != ?"));
        else
            queryStr = queryStr.arg(QStringLiteral("AND state = ?"));

        queryStr = queryStr.arg(static_cast<int>(orderColumn) + 1);

        auto query = prepare(queryStr.arg(limit));
        query.addBindValue(characterId);

        if (!includeActive || !includeNotFulfilled)
        {
            if (includeActive)
            {
                query.addBindValue(static_cast<int>(MarketOrder::State::Active));
                query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));
            }
            else if (includeNotFulfilled)
            {
                query.addBindValue(static_cast<int>(MarketOrder::State::Active));
            }
            else
            {
                query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));
            }
        }

        DatabaseUtils::execQuery(query);

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
        {
            SingleAggrData data;
            data.mCount = query.value(1).toUInt();
            data.mPriceSum = query.value(2).toDouble();
            data.mVolume = query.value(3).toUInt();

            result.emplace_back(std::make_pair(query.value(0).value<quint64>(), std::move(data)));
        }

        return result;
    }

    MarketOrderRepository::OrderStateMap MarketOrderRepository::getOrderStates(Character::IdType characterId) const
    {
        OrderStateMap result;

        auto query = prepare(QStringLiteral("SELECT %1, state, volume_remaining, first_seen, last_seen, delta, issued, duration, custom_location_id, notes, color_tag FROM %2 WHERE character_id = ?")
            .arg(getIdColumn())
            .arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);

        while (query.next())
        {
            OrderState state;
            state.mState = static_cast<MarketOrder::State>(query.value(1).toInt());
            state.mVolumeRemaining = query.value(2).toUInt();
            state.mFirstSeen = query.value(3).toDateTime();
            state.mFirstSeen.setTimeSpec(Qt::UTC);
            state.mLastSeen = query.value(4).toDateTime();
            state.mLastSeen.setTimeSpec(Qt::UTC);
            state.mExpiry = query.value(6).toDateTime().addDays(query.value(7).toInt());
            state.mExpiry.setTimeSpec(Qt::UTC);
            state.mDelta = query.value(5).toInt();
            state.mNotes = query.value(9).toString();
            state.mColorTag = query.value(10).toString();

            if (!query.value(8).isNull())
                state.mCustomStation = query.value(8).toUInt();

            result.emplace(query.value(0).value<MarketOrder::IdType>(), std::move(state));
        }

        return result;
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchForCharacter(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE character_id = ?").arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchForCharacter(Character::IdType characterId, MarketOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE character_id = ? AND type = ?").arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, static_cast<int>(type));

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchForCorporation(uint corporationId, MarketOrder::Type type) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE corporation_id = ? AND type = ?").arg(getTableName()));
        query.bindValue(0, corporationId);
        query.bindValue(1, static_cast<int>(type));

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchArchivedForCharacter(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE character_id = ? AND last_seen IS NOT NULL").arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchArchivedForCorporation(uint corporationId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE corporation_id = ? AND last_seen IS NOT NULL").arg(getTableName()));
        query.bindValue(0, corporationId);

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchFulfilled(const QDate &from, const QDate &to) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE first_seen BETWEEN ? AND ? AND last_seen IS NOT NULL AND state = ?").arg(getTableName()));
        query.addBindValue(from);
        query.addBindValue(to);
        query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchFulfilledForCharacter(const QDate &from, const QDate &to, Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE character_id = ? AND first_seen BETWEEN ? AND ? AND last_seen IS NOT NULL AND state = ?").arg(getTableName()));
        query.addBindValue(characterId);
        query.addBindValue(from);
        query.addBindValue(to);
        query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));

        DatabaseUtils::execQuery(query);

        return populate(query);
    }

    MarketOrderRepository::EntityList MarketOrderRepository::fetchFulfilledForCorporation(const QDate &from, const QDate &to, uint corporationId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE corporation_id = ? AND first_seen BETWEEN ? AND ? AND last_seen IS NOT NULL AND state = ?").arg(getTableName()));
        query.addBindValue(corporationId);
        query.addBindValue(from);
        query.addBindValue(to);
        query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));

        return populate(query);
    }

    TypeLocationPairs MarketOrderRepository::fetchActiveTypes() const
    {
        auto query = prepare(QStringLiteral("SELECT type_id, location_id FROM %1 WHERE state = ?").arg(getTableName()));
        query.bindValue(0, static_cast<int>(MarketOrder::State::Active));

        DatabaseUtils::execQuery(query);

        TypeLocationPairs result;

        while (query.next())
            result.insert(std::make_pair(query.value(0).value<EveType::IdType>(), query.value(1).toULongLong()));

        return result;
    }

    void MarketOrderRepository::archive(const std::vector<MarketOrder::IdType> &ids) const
    {
        const auto baseQuery = QStringLiteral("UPDATE %1 SET "
            "last_seen = min(strftime('%Y-%m-%dT%H:%M:%f', first_seen, duration || ' days'), strftime('%Y-%m-%dT%H:%M:%f', 'now')),"
            "state = ?,"
            "delta = 0 "
            "WHERE %2 IN (%3)").arg(getTableName()).arg(getIdColumn());

        execBoundValueBatch(maxSqliteBoundVariables - 1, baseQuery, ids, [](auto &query) {
            query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));
        });
    }

    void MarketOrderRepository::fulfill(const std::vector<MarketOrder::IdType> &ids) const
    {
        const auto baseQuery = QStringLiteral("UPDATE %1 SET "
            "last_seen = ?,"
            "state = ?,"
            "delta = volume_remaining,"
            "volume_remaining = 0 "
            "WHERE %2 IN (%3)").arg(getTableName()).arg(getIdColumn());

        execBoundValueBatch(maxSqliteBoundVariables - 2, baseQuery, ids, [](auto &query) {
            query.addBindValue(QDateTime::currentDateTimeUtc());
            query.addBindValue(static_cast<int>(MarketOrder::State::Fulfilled));
        });
    }

    void MarketOrderRepository::deleteOldEntries(const QDateTime &from) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE last_seen < ?").arg(getTableName()));
        query.bindValue(0, from);

        DatabaseUtils::execQuery(query);
    }

    void MarketOrderRepository::setNotes(MarketOrder::IdType id, const QString &notes) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET notes = ? WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, notes);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void MarketOrderRepository::setStation(MarketOrder::IdType orderId, uint stationId) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET custom_location_id = ? WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, (stationId != 0) ? (stationId) : (QVariant{QVariant::UInt}));
        query.bindValue(1, orderId);

        DatabaseUtils::execQuery(query);
    }

    void MarketOrderRepository::setColorTag(MarketOrder::IdType orderId, const QColor &color) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET color_tag = ? WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, (color.isValid()) ? (color.name()) : (QVariant{QVariant::String}));
        query.bindValue(1, orderId);

        DatabaseUtils::execQuery(query);
    }

    QStringList MarketOrderRepository::getColumns() const
    {
        return QStringList{}
            << QStringLiteral("id")
            << QStringLiteral("character_id")
            << QStringLiteral("location_id")
            << QStringLiteral("volume_entered")
            << QStringLiteral("volume_remaining")
            << QStringLiteral("min_volume")
            << QStringLiteral("delta")
            << QStringLiteral("state")
            << QStringLiteral("type_id")
            << QStringLiteral("range")
            << QStringLiteral("account_key")
            << QStringLiteral("duration")
            << QStringLiteral("escrow")
            << QStringLiteral("price")
            << QStringLiteral("type")
            << QStringLiteral("issued")
            << QStringLiteral("first_seen")
            << QStringLiteral("last_seen")
            << QStringLiteral("corporation_id")
            << QStringLiteral("notes")
            << QStringLiteral("custom_location_id")
            << QStringLiteral("color_tag");
    }

    void MarketOrderRepository::bindValues(const MarketOrder &entity, QSqlQuery &query) const
    {
        if (entity.getId() != MarketOrder::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":character_id"), entity.getCharacterId());
        query.bindValue(QStringLiteral(":location_id"), entity.getStationId());
        query.bindValue(QStringLiteral(":volume_entered"), entity.getVolumeEntered());
        query.bindValue(QStringLiteral(":volume_remaining"), entity.getVolumeRemaining());
        query.bindValue(QStringLiteral(":min_volume"), entity.getMinVolume());
        query.bindValue(QStringLiteral(":delta"), entity.getDelta());
        query.bindValue(QStringLiteral(":state"), static_cast<int>(entity.getState()));
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
        query.bindValue(QStringLiteral(":range"), entity.getRange());
        query.bindValue(QStringLiteral(":account_key"), entity.getAccountKey());
        query.bindValue(QStringLiteral(":duration"), entity.getDuration());
        query.bindValue(QStringLiteral(":escrow"), entity.getEscrow());
        query.bindValue(QStringLiteral(":price"), entity.getPrice());
        query.bindValue(QStringLiteral(":type"), static_cast<int>(entity.getType()));
        query.bindValue(QStringLiteral(":issued"), entity.getIssued());
        query.bindValue(QStringLiteral(":first_seen"), entity.getFirstSeen());
        query.bindValue(QStringLiteral(":last_seen"), entity.getLastSeen());
        query.bindValue(QStringLiteral(":corporation_id"), entity.getCorporationId());
        query.bindValue(QStringLiteral(":notes"), entity.getNotes());
        query.bindValue(QStringLiteral(":custom_location_id"), (entity.getCustomStationId()) ? (*entity.getCustomStationId()) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":color_tag"), (entity.getColorTag().isValid()) ? (entity.getColorTag().name()) : (QVariant{QVariant::String}));
    }

    void MarketOrderRepository::bindPositionalValues(const MarketOrder &entity, QSqlQuery &query) const
    {
        if (entity.getId() != MarketOrder::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getStationId());
        query.addBindValue(entity.getVolumeEntered());
        query.addBindValue(entity.getVolumeRemaining());
        query.addBindValue(entity.getMinVolume());
        query.addBindValue(entity.getDelta());
        query.addBindValue(static_cast<int>(entity.getState()));
        query.addBindValue(entity.getTypeId());
        query.addBindValue(entity.getRange());
        query.addBindValue(entity.getAccountKey());
        query.addBindValue(entity.getDuration());
        query.addBindValue(entity.getEscrow());
        query.addBindValue(entity.getPrice());
        query.addBindValue(static_cast<int>(entity.getType()));
        query.addBindValue(entity.getIssued());
        query.addBindValue(entity.getFirstSeen());
        query.addBindValue(entity.getLastSeen());
        query.addBindValue(entity.getCorporationId());
        query.addBindValue(entity.getNotes());
        query.addBindValue((entity.getCustomStationId()) ? (*entity.getCustomStationId()) : (QVariant{QVariant::ULongLong}));
        query.addBindValue((entity.getColorTag().isValid()) ? (entity.getColorTag().name()) : (QVariant{QVariant::String}));
    }

    template<class Binder>
    void MarketOrderRepository::execBoundValueBatch(size_t maxBatchSize,
                                                    const QString &baseQuery,
                                                    const std::vector<MarketOrder::IdType> &ids,
                                                    const Binder &valueBinder) const
    {
        const auto batches = ids.size() / maxBatchSize;

        QStringList list;
        for (auto i = 0u; i < std::min(maxBatchSize, ids.size()); ++i)
            list << QStringLiteral("?");

        auto query = prepare(baseQuery.arg(list.join(QStringLiteral(", "))));

        auto executeBatch = [&](auto begin, auto end) {
            for (auto it = begin; it != end; ++it)
                query.addBindValue(*it);

            DatabaseUtils::execQuery(query);
        };

        for (auto batch = 0u; batch < batches; ++batch)
        {
            valueBinder(query);

            const auto end = std::next(std::begin(ids), (batch + 1) * maxBatchSize);
            executeBatch(std::next(std::begin(ids), batch * maxBatchSize), end);
        }

        const auto reminderBegin = std::next(std::begin(ids), batches * maxBatchSize);
        if (reminderBegin != std::end(ids))
        {
            list.clear();
            for (auto it = reminderBegin; it != std::end(ids); ++it)
                list << "?";

            query = prepare(baseQuery.arg(list.join(QStringLiteral(", "))));

            valueBinder(query);
            executeBatch(reminderBegin, std::end(ids));
        }
    }

    MarketOrderRepository::EntityList MarketOrderRepository::populate(QSqlQuery &query) const
    {
        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }
}
