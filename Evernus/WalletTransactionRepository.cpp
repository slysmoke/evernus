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

#include "WalletTransactionRepository.h"

namespace Evernus
{
    WalletTransactionRepository::WalletTransactionRepository(bool corp, const DatabaseConnectionProvider &connectionProvider)
        : Repository{connectionProvider}
        , mCorp{corp}
    {
    }

    QString WalletTransactionRepository::getTableName() const
    {
        return (mCorp) ? (QStringLiteral("corp_wallet_transactions")) : (QStringLiteral("wallet_transactions"));
    }

    QString WalletTransactionRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    WalletTransactionRepository::EntityPtr WalletTransactionRepository::populate(const QSqlRecord &record) const
    {
        auto timestamp = record.value(QStringLiteral("timestamp")).toDateTime();
        timestamp.setTimeSpec(Qt::UTC);

        auto walletTransaction = std::make_shared<WalletTransaction>(record.value(QStringLiteral("id")).value<WalletTransaction::IdType>());
        walletTransaction->setCharacterId(record.value(QStringLiteral("character_id")).value<Character::IdType>());
        walletTransaction->setTimestamp(timestamp);
        walletTransaction->setQuantity(record.value(QStringLiteral("quantity")).toUInt());
        walletTransaction->setTypeId(record.value(QStringLiteral("type_id")).value<EveType::IdType>());
        walletTransaction->setPrice(record.value(QStringLiteral("price")).toDouble());
        walletTransaction->setClientId(record.value(QStringLiteral("client_id")).toULongLong());
        walletTransaction->setLocationId(record.value(QStringLiteral("location_id")).toULongLong());
        walletTransaction->setType(static_cast<WalletTransaction::Type>(record.value(QStringLiteral("type")).toInt()));
        walletTransaction->setJournalId(record.value(QStringLiteral("journal_id")).value<WalletJournalEntry::IdType>());
        walletTransaction->setCorporationId(record.value(QStringLiteral("corporation_id")).toULongLong());
        walletTransaction->setIgnored(record.value(QStringLiteral("ignored")).toBool());
        walletTransaction->setNew(false);

        return walletTransaction;
    }

    void WalletTransactionRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "character_id BIGINT NOT NULL %2,"
            "timestamp DATETIME NOT NULL,"
            "quantity INTEGER NOT NULL,"
            "type_id INTEGER NOT NULL,"
            "price NUMERIC NOT NULL,"
            "client_id BIGINT NOT NULL,"
            "location_id BIGINT NOT NULL,"
            "type TINYINT NOT NULL,"
            "journal_id BIGINT NOT NULL,"
            "corporation_id BIGINT NOT NULL,"
            "ignored TINYINT NOT NULL"
        ")").arg(getTableName()).arg(
            (mCorp) ? (QString{}) : (QStringLiteral("REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE").arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()))));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)").arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_id ON %1(type_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_type_id_character ON %1(type_id, character_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_timestamp ON %1(timestamp)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_timestamp_type ON %1(character_id, timestamp, type)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_timestamp_type_type_id ON %1(character_id, timestamp, type, type_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_ignored_timestamp ON %1(ignored, timestamp)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_ignored_timestamp ON %1(character_id, ignored, timestamp)").arg(getTableName()));

        try
        {
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_timestamp_type ON %1(corporation_id, timestamp, type)").arg(getTableName()));
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_timestamp_type_type_id ON %1(corporation_id, timestamp, type, type_id)").arg(getTableName()));
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_ignored_timestamp ON %1(corporation_id, ignored, timestamp)").arg(getTableName()));
        }
        catch (const std::exception &)
        {
            // ignore - versions < 1.9 do not have this column
            qDebug() << "SQL errors ignored";
        }
    }

    WalletTransaction::IdType WalletTransactionRepository::getLatestEntryId(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT MAX(%1) FROM %2 WHERE character_id = ?").arg(getIdColumn()).arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);

        query.next();
        return query.value(0).value<WalletTransaction::IdType>();
    }

    void WalletTransactionRepository::setIgnored(WalletTransaction::IdType id, bool ignored) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET ignored = ? WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, ignored);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void WalletTransactionRepository::deleteOldEntries(const QDateTime &from) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE timestamp < ?").arg(getTableName()));
        query.bindValue(0, from);

        DatabaseUtils::execQuery(query);
    }

    void WalletTransactionRepository::deleteAll() const
    {
        exec(QStringLiteral("DELETE FROM %1").arg(getTableName()));
    }

    WalletTransactionRepository::EntityList WalletTransactionRepository
    ::fetchInRange(const QDateTime &from,
                   const QDateTime &till,
                   EntryType type,
                   EveType::IdType typeId) const
    {
        QString queryStr;
        if (type == EntryType::All)
            queryStr = "SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ?";
        else
            queryStr = "SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ? AND type = ?";

        if (typeId != EveType::invalidId)
            queryStr += " AND type_id = ?";

        auto query = prepare(queryStr.arg(getTableName()));
        query.addBindValue(from);
        query.addBindValue(till);

        if (type != EntryType::All)
            query.addBindValue(static_cast<int>((type == EntryType::Buy) ? (WalletTransaction::Type::Buy) : (WalletTransaction::Type::Sell)));

        if (typeId != EveType::invalidId)
            query.addBindValue(typeId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();

        EntityList result;
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    WalletTransactionRepository::EntityList WalletTransactionRepository
    ::fetchForCharacterInRange(Character::IdType characterId,
                               const QDateTime &from,
                               const QDateTime &till,
                               EntryType type,
                               EveType::IdType typeId) const
    {
        return fetchForColumnInRange(characterId, from, till, type, typeId, QStringLiteral("character_id"));
    }

    WalletTransactionRepository::EntityList WalletTransactionRepository
    ::fetchForCorporationInRange(quint64 corporationId,
                                 const QDateTime &from,
                                 const QDateTime &till,
                                 EntryType type,
                                 EveType::IdType typeId) const
    {
        return fetchForColumnInRange(corporationId, from, till, type, typeId, QStringLiteral("corporation_id"));
    }

    WalletTransactionRepository::EntityList WalletTransactionRepository::fetchForTypeId(EveType::IdType typeId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type_id = ?").arg(getTableName()));
        query.bindValue(0, typeId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();

        EntityList result;
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    WalletTransactionRepository::EntityList WalletTransactionRepository
    ::fetchForTypeIdAndCharacter(EveType::IdType typeId, Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE type_id = ? AND character_id = ?").arg(getTableName()));
        query.bindValue(0, typeId);
        query.bindValue(1, characterId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();

        EntityList result;
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    QStringList WalletTransactionRepository::getColumns() const
    {
        return {
            QStringLiteral("id"),
            QStringLiteral("character_id"),
            QStringLiteral("timestamp"),
            QStringLiteral("quantity"),
            QStringLiteral("type_id"),
            QStringLiteral("price"),
            QStringLiteral("client_id"),
            QStringLiteral("location_id"),
            QStringLiteral("type"),
            QStringLiteral("journal_id"),
            QStringLiteral("corporation_id"),
            QStringLiteral("ignored"),
        };
    }

    void WalletTransactionRepository::bindValues(const WalletTransaction &entity, QSqlQuery &query) const
    {
        if (entity.getId() != WalletTransaction::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(QStringLiteral(":character_id"), entity.getCharacterId());
        query.bindValue(QStringLiteral(":timestamp"), entity.getTimestamp());
        query.bindValue(QStringLiteral(":quantity"), entity.getQuantity());
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
        query.bindValue(QStringLiteral(":price"), entity.getPrice());
        query.bindValue(QStringLiteral(":client_id"), entity.getClientId());
        query.bindValue(QStringLiteral(":location_id"), entity.getLocationId());
        query.bindValue(QStringLiteral(":type"), static_cast<int>(entity.getType()));
        query.bindValue(QStringLiteral(":journal_id"), entity.getJournalId());
        query.bindValue(QStringLiteral(":corporation_id"), entity.getCorporationId());
        query.bindValue(QStringLiteral(":ignored"), entity.isIgnored());
    }

    void WalletTransactionRepository::bindPositionalValues(const WalletTransaction &entity, QSqlQuery &query) const
    {
        if (entity.getId() != WalletTransaction::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getTimestamp());
        query.addBindValue(entity.getQuantity());
        query.addBindValue(entity.getTypeId());
        query.addBindValue(entity.getPrice());
        query.addBindValue(entity.getClientId());
        query.addBindValue(entity.getLocationId());
        query.addBindValue(static_cast<int>(entity.getType()));
        query.addBindValue(entity.getJournalId());
        query.addBindValue(entity.getCorporationId());
        query.addBindValue(entity.isIgnored());
    }

    template<class T>
    WalletTransactionRepository::EntityList WalletTransactionRepository::fetchForColumnInRange(T id,
                                                                                               const QDateTime &from,
                                                                                               const QDateTime &till,
                                                                                               EntryType type,
                                                                                               EveType::IdType typeId,
                                                                                               const QString &column) const
    {
        QString queryStr;
        if (type == EntryType::All)
            queryStr = "SELECT * FROM %2 WHERE %1 = ? AND timestamp BETWEEN ? AND ?";
        else
            queryStr = "SELECT * FROM %2 WHERE %1 = ? AND timestamp BETWEEN ? AND ? AND type = ?";

        if (typeId != EveType::invalidId)
            queryStr += " AND type_id = ?";

        auto query = prepare(queryStr.arg(column).arg(getTableName()));
        query.addBindValue(id);
        query.addBindValue(from);
        query.addBindValue(till);

        if (type != EntryType::All)
            query.addBindValue(static_cast<int>((type == EntryType::Buy) ? (WalletTransaction::Type::Buy) : (WalletTransaction::Type::Sell)));

        if (typeId != EveType::invalidId)
            query.addBindValue(typeId);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();

        EntityList result;
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }
}
