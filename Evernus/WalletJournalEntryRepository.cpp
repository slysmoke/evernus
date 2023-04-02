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

#include "WalletJournalEntryRepository.h"

namespace Evernus
{
    WalletJournalEntryRepository::WalletJournalEntryRepository(bool corp, const DatabaseConnectionProvider &connectionProvider)
        : Repository{connectionProvider}
        , mCorp{corp}
    {
    }

    QString WalletJournalEntryRepository::getTableName() const
    {
        return (mCorp) ? (QStringLiteral("corp_wallet_journal")) : (QStringLiteral("wallet_journal"));
    }

    QString WalletJournalEntryRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    WalletJournalEntryRepository::EntityPtr WalletJournalEntryRepository::populate(const QSqlRecord &record) const
    {
        const auto taxReceiverId = record.value(QStringLiteral("tax_receiver_id"));
        const auto extraInfoId = record.value(QStringLiteral("extra_info_id"));
        const auto firstPartyId = record.value(QStringLiteral("first_party_id"));
        const auto secondPartyId = record.value(QStringLiteral("second_party_id"));
        const auto taxAmount = record.value(QStringLiteral("tax_amount"));
        const auto balance = record.value(QStringLiteral("balance"));
        const auto amount = record.value(QStringLiteral("amount"));
        const auto contextId = record.value(QStringLiteral("context_id"));

        auto timestamp = record.value(QStringLiteral("timestamp")).toDateTime();
        timestamp.setTimeSpec(Qt::UTC);

        auto walletJournalEntry = std::make_shared<WalletJournalEntry>(record.value(getIdColumn()).value<WalletJournalEntry::IdType>());
        walletJournalEntry->setCharacterId(record.value(QStringLiteral("character_id")).value<Character::IdType>());
        walletJournalEntry->setTimestamp(timestamp);
        walletJournalEntry->setFirstPartyId((firstPartyId.isNull()) ? (WalletJournalEntry::PartyIdType{}) : (firstPartyId.toULongLong()));
        walletJournalEntry->setSecondPartyId((secondPartyId.isNull()) ? (WalletJournalEntry::PartyIdType{}) : (secondPartyId.toULongLong()));
        walletJournalEntry->setAmount((amount.isNull()) ? (WalletJournalEntry::ISKType{}) : (amount.toDouble()));
        walletJournalEntry->setBalance((balance.isNull()) ? (WalletJournalEntry::ISKType{}) : (balance.toDouble()));
        walletJournalEntry->setReason(record.value(QStringLiteral("reason")).toString());
        walletJournalEntry->setTaxReceiverId((taxReceiverId.isNull()) ? (WalletJournalEntry::TaxReceiverType{}) : (taxReceiverId.toULongLong()));
        walletJournalEntry->setTaxAmount((taxAmount.isNull()) ? (WalletJournalEntry::ISKType{}) : (taxAmount.toDouble()));
        walletJournalEntry->setCorporationId(record.value(QStringLiteral("corporation_id")).toULongLong());
        walletJournalEntry->setIgnored(record.value(QStringLiteral("ignored")).toBool());
        walletJournalEntry->setRefType(record.value(QStringLiteral("ref_type")).toString());
        walletJournalEntry->setContextId((contextId.isNull()) ? (WalletJournalEntry::ContextIdType{}) : (contextId.toULongLong()));
        walletJournalEntry->setContextIdType(record.value(QStringLiteral("context_id_type")).toString());
        walletJournalEntry->setNew(false);

        return walletJournalEntry;
    }

    void WalletJournalEntryRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "character_id BIGINT NOT NULL %2,"
            "timestamp DATETIME NOT NULL,"
            "first_party_id BIGINT NULL,"
            "second_party_id BIGINT NULL,"
            "amount NUMERIC NULL,"
            "balance NUMERIC NULL,"
            "reason TEXT NULL,"
            "tax_receiver_id BIGINT NULL,"
            "tax_amount NUMERIC NULL,"
            "corporation_id BIGINT NOT NULL,"
            "context_id BIGINT NULL,"
            "context_id_type TEXT NULL,"
            "ignored TINYINT NOT NULL,"
            "ref_type TEXT NULL"
        ")").arg(getTableName()).arg(
            (mCorp) ? (QString{}) : (QStringLiteral("REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE").arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()))));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)").arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_timestamp ON %1(timestamp)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_timestamp ON %1(character_id, timestamp)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_timestamp_amount ON %1(character_id, timestamp, amount)").arg(getTableName()));

        try
        {
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_timestamp ON %1(corporation_id, timestamp)").arg(getTableName()));
            exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_corporation_timestamp_amount ON %1(corporation_id, timestamp, amount)").arg(getTableName()));
        }
        catch (const std::exception &)
        {
            // ignore - versions < 1.9 do not have this column
            qDebug() << "SQL errors ignored";
        }
    }

    WalletJournalEntry::IdType WalletJournalEntryRepository::getLatestEntryId(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("SELECT MAX(%1) FROM %2 WHERE character_id = ?").arg(getIdColumn()).arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);

        query.next();
        return query.value(0).value<WalletJournalEntry::IdType>();
    }

    void WalletJournalEntryRepository::setIgnored(WalletJournalEntry::IdType id, bool ignored) const
    {
        auto query = prepare(QStringLiteral("UPDATE %1 SET ignored = ? WHERE %2 = ?").arg(getTableName()).arg(getIdColumn()));
        query.bindValue(0, ignored);
        query.bindValue(1, id);

        DatabaseUtils::execQuery(query);
    }

    void WalletJournalEntryRepository::deleteOldEntries(const QDateTime &from) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE timestamp < ?").arg(getTableName()));
        query.bindValue(0, from);

        DatabaseUtils::execQuery(query);
    }

    void WalletJournalEntryRepository::deleteAll() const
    {
        exec(QStringLiteral("DELETE FROM %1").arg(getTableName()));
    }

    WalletJournalEntryRepository::EntityList WalletJournalEntryRepository
    ::fetchInRange(const QDateTime &from, const QDateTime &till, EntryType type) const
    {
        QString queryStr;
        switch (type) {
        case EntryType::Incomig:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ? AND amount >= 0");
            break;
        case EntryType::Outgoing:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ? AND amount < 0");
            break;
        default:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ?");
        }

        auto query = prepare(queryStr.arg(getTableName()));
        query.addBindValue(from);
        query.addBindValue(till);

        DatabaseUtils::execQuery(query);

        const auto size = query.size();

        EntityList result;
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    WalletJournalEntryRepository::EntityList WalletJournalEntryRepository
    ::fetchForCharacterInRange(Character::IdType characterId, const QDateTime &from, const QDateTime &till, EntryType type) const
    {
        return fetchForColumnInRange(characterId, from, till, type, QStringLiteral("character_id"));
    }

    WalletJournalEntryRepository::EntityList WalletJournalEntryRepository
    ::fetchForCorporationInRange(quint64 corporationId, const QDateTime &from, const QDateTime &till, EntryType type) const
    {
        return fetchForColumnInRange(corporationId, from, till, type, QStringLiteral("corporation_id"));
    }

    QStringList WalletJournalEntryRepository::getColumns() const
    {
        return {
            QStringLiteral("id"),
            QStringLiteral("character_id"),
            QStringLiteral("timestamp"),
            QStringLiteral("first_party_id"),
            QStringLiteral("second_party_id"),
            QStringLiteral("amount"),
            QStringLiteral("balance"),
            QStringLiteral("reason"),
            QStringLiteral("tax_receiver_id"),
            QStringLiteral("tax_amount"),
            QStringLiteral("corporation_id"),
            QStringLiteral("ignored"),
            QStringLiteral("ref_type"),
            QStringLiteral("context_id"),
            QStringLiteral("context_id_type"),
        };
    }

    void WalletJournalEntryRepository::bindValues(const WalletJournalEntry &entity, QSqlQuery &query) const
    {
        const auto taxReceiverId = entity.getTaxReceiverId();
        const auto firstPartyId = entity.getFirstPartyId();
        const auto secondPartyId = entity.getSecondPartyId();
        const auto taxAmount = entity.getTaxAmount();
        const auto amount = entity.getAmount();
        const auto balance = entity.getBalance();
        const auto contextId = entity.getContextId();

        if (entity.getId() != WalletJournalEntry::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":character_id"), entity.getCharacterId());
        query.bindValue(QStringLiteral(":timestamp"), entity.getTimestamp());
        query.bindValue(QStringLiteral(":first_party_id"), (firstPartyId) ? (*firstPartyId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":second_party_id"), (secondPartyId) ? (*secondPartyId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":amount"), (amount) ? (*amount) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":balance"), (balance) ? (*balance) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":reason"), entity.getReason());
        query.bindValue(QStringLiteral(":tax_receiver_id"), (taxReceiverId) ? (*taxReceiverId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":tax_amount"), (taxAmount) ? (*taxAmount) : (QVariant{QVariant::Double}));
        query.bindValue(QStringLiteral(":corporation_id"), entity.getCorporationId());
        query.bindValue(QStringLiteral(":ignored"), entity.isIgnored());
        query.bindValue(QStringLiteral(":ref_type"), entity.getRefType());
        query.bindValue(QStringLiteral(":context_id"), (contextId) ? (*contextId) : (QVariant{QVariant::ULongLong}));
        query.bindValue(QStringLiteral(":context_id_type"), entity.getContextIdType());
    }

    void WalletJournalEntryRepository::bindPositionalValues(const WalletJournalEntry &entity, QSqlQuery &query) const
    {
        const auto taxReceiverId = entity.getTaxReceiverId();
        const auto firstPartyId = entity.getFirstPartyId();
        const auto secondPartyId = entity.getSecondPartyId();
        const auto taxAmount = entity.getTaxAmount();
        const auto amount = entity.getAmount();
        const auto balance = entity.getBalance();
        const auto contextId = entity.getContextId();

        if (entity.getId() != WalletJournalEntry::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getTimestamp());
        query.addBindValue((firstPartyId) ? (*firstPartyId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue((secondPartyId) ? (*secondPartyId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue((amount) ? (*amount) : (QVariant{QVariant::Double}));
        query.addBindValue((balance) ? (*balance) : (QVariant{QVariant::Double}));
        query.addBindValue(entity.getReason());
        query.addBindValue((taxReceiverId) ? (*taxReceiverId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue((taxAmount) ? (*taxAmount) : (QVariant{QVariant::Double}));
        query.addBindValue(entity.getCorporationId());
        query.addBindValue(entity.isIgnored());
        query.addBindValue(entity.getRefType());
        query.addBindValue((contextId) ? (*contextId) : (QVariant{QVariant::ULongLong}));
        query.addBindValue(entity.getContextIdType());
    }

    template<class T>
    WalletJournalEntryRepository::EntityList WalletJournalEntryRepository::fetchForColumnInRange(T id,
                                                                                                 const QDateTime &from,
                                                                                                 const QDateTime &till,
                                                                                                 EntryType type,
                                                                                                 const QString &column) const
    {
        QString queryStr;
        switch (type) {
        case EntryType::Incomig:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE %2 = ? AND timestamp BETWEEN ? AND ? AND amount >= 0");
            break;
        case EntryType::Outgoing:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE %2 = ? AND timestamp BETWEEN ? AND ? AND amount < 0");
            break;
        default:
            queryStr = QStringLiteral("SELECT * FROM %1 WHERE %2 = ? AND timestamp BETWEEN ? AND ?");
        }

        auto query = prepare(queryStr.arg(getTableName()).arg(column));
        query.addBindValue(id);
        query.addBindValue(from);
        query.addBindValue(till);

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
