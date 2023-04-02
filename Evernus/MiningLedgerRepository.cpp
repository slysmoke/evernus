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
#include "MiningLedgerRepository.h"

namespace Evernus
{
    QString MiningLedgerRepository::getTableName() const
    {
        return QStringLiteral("mining_ledger");
    }

    QString MiningLedgerRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    MiningLedgerRepository::EntityPtr MiningLedgerRepository::populate(const QSqlRecord &record) const
    {
        auto ledger = std::make_shared<MiningLedger>(record.value(getIdColumn()).value<MiningLedger::IdType>());
        ledger->setCharacterId(record.value(QStringLiteral("character_id")).value<Character::IdType>());
        ledger->setDate(record.value(QStringLiteral("date")).toDate());
        ledger->setQuantity(record.value(QStringLiteral("quantity")).toUInt());
        ledger->setSolarSystemId(record.value(QStringLiteral("solar_system_id")).toUInt());
        ledger->setTypeId(record.value(QStringLiteral("type_id")).value<EveType::IdType>());

        return ledger;
    }

    void MiningLedgerRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "character_id BIGINT NOT NULL REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE,"
            "date DATE NOT NULL,"
            "quantity INTEGER NOT NULL,"
            "solar_system_id INTEGER NOT NULL,"
            "type_id INTEGER NOT NULL"
        ")").arg(getTableName()).arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)").arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_character_date ON %1(character_id, date)").arg(getTableName()));
    }

    void MiningLedgerRepository::removeForCharacter(Character::IdType characterId) const
    {
        auto query = prepare(QStringLiteral("DELETE FROM %1 WHERE character_id = ?").arg(getTableName()));
        query.bindValue(0, characterId);

        DatabaseUtils::execQuery(query);
    }

    MiningLedgerRepository::EntityList MiningLedgerRepository::fetchForCharacter(Character::IdType characterId,
                                                                                 const QDate &from,
                                                                                 const QDate &to) const
    {
        auto query = prepare(QStringLiteral("SELECT * FROM %1 WHERE character_id = ? AND date BETWEEN ? AND ?").arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, from);
        query.bindValue(2, to);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    MiningLedgerRepository::TypeQuantityMap MiningLedgerRepository::fetchTypesForCharacter(Character::IdType characterId,
                                                                                           const QDate &from,
                                                                                           const QDate &to) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT type_id, SUM(quantity) FROM %1 WHERE character_id = ? AND date BETWEEN ? AND ? GROUP BY type_id"
        ).arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, from);
        query.bindValue(2, to);

        DatabaseUtils::execQuery(query);

        TypeQuantityMap result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace(query.value(0).value<EveType::IdType>(), query.value(1).toULongLong());

        return result;
    }

    MiningLedgerRepository::SolarSystemQuantityMap MiningLedgerRepository
    ::fetchSolarSystemsForCharacter(Character::IdType characterId,
                                    const QDate &from,
                                    const QDate &to) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT solar_system_id, SUM(quantity) FROM %1 WHERE character_id = ? AND date BETWEEN ? AND ? GROUP BY solar_system_id"
        ).arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, from);
        query.bindValue(2, to);

        DatabaseUtils::execQuery(query);

        SolarSystemQuantityMap result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace(query.value(0).toUInt(), query.value(1).toULongLong());

        return result;
    }

    MiningLedgerRepository::SolarSystemTypeAggregatedData MiningLedgerRepository
    ::fetchAggregatedDataForCharacter(Character::IdType characterId,
                                      const QDate &from,
                                      const QDate &to) const
    {
        auto query = prepare(QStringLiteral(
            "SELECT solar_system_id, type_id, SUM(quantity) FROM %1 WHERE character_id = ? AND date BETWEEN ? AND ? GROUP BY solar_system_id, type_id"
        ).arg(getTableName()));
        query.bindValue(0, characterId);
        query.bindValue(1, from);
        query.bindValue(2, to);

        DatabaseUtils::execQuery(query);

        SolarSystemTypeAggregatedData result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
        {
            result.emplace_back(SolarSystemTypeData{
                query.value(2).toULongLong(),
                query.value(1).value<EveType::IdType>(),
                query.value(0).toUInt()
            });
        }

        return result;
    }

    QStringList MiningLedgerRepository::getColumns() const
    {
        return {
            getIdColumn(),
            QStringLiteral("character_id"),
            QStringLiteral("date"),
            QStringLiteral("quantity"),
            QStringLiteral("solar_system_id"),
            QStringLiteral("type_id"),
        };
    }

    void MiningLedgerRepository::bindValues(const MiningLedger &entity, QSqlQuery &query) const
    {
        if (entity.getId() != MiningLedger::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":character_id"), entity.getCharacterId());
        query.bindValue(QStringLiteral(":date"), entity.getDate());
        query.bindValue(QStringLiteral(":quantity"), entity.getQuantity());
        query.bindValue(QStringLiteral(":solar_system_id"), entity.getSolarSystemId());
        query.bindValue(QStringLiteral(":type_id"), entity.getTypeId());
    }

    void MiningLedgerRepository::bindPositionalValues(const MiningLedger &entity, QSqlQuery &query) const
    {
        if (entity.getId() != MiningLedger::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getDate());
        query.addBindValue(entity.getQuantity());
        query.addBindValue(entity.getSolarSystemId());
        query.addBindValue(entity.getTypeId());
    }
}
