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

#include "AssetValueSnapshotRepository.h"

namespace Evernus
{
    QString AssetValueSnapshotRepository::getTableName() const
    {
        return "asset_value_snapshots";
    }

    QString AssetValueSnapshotRepository::getIdColumn() const
    {
        return "id";
    }

    AssetValueSnapshotRepository::EntityPtr AssetValueSnapshotRepository::populate(const QSqlRecord &record) const
    {
        auto dt = record.value("timestamp").toDateTime();
        dt.setTimeSpec(Qt::UTC);

        auto assetValueSnapshot
            = std::make_shared<AssetValueSnapshot>(record.value("id").value<AssetValueSnapshot::IdType>(), record.value("balance").toDouble());
        assetValueSnapshot->setTimestamp(dt);
        assetValueSnapshot->setCharacterId(record.value("character_id").value<Character::IdType>());
        assetValueSnapshot->setNew(false);

        return assetValueSnapshot;
    }

    void AssetValueSnapshotRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY ASC,"
            "timestamp DATETIME NOT NULL,"
            "character_id BIGINT NOT NULL REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE,"
            "balance DOUBLE NOT NULL"
        ")"}.arg(getTableName()).arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()));

        exec(QString{"CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)"}.arg(getTableName()).arg(characterRepo.getTableName()));
        exec(QString{"CREATE INDEX IF NOT EXISTS %1_timestamp ON %1(timestamp)"}.arg(getTableName()));
        exec(QString{"CREATE UNIQUE INDEX IF NOT EXISTS %1_character_timestamp ON %1(character_id, timestamp)"}.arg(getTableName()));
    }

    AssetValueSnapshotRepository::EntityList AssetValueSnapshotRepository
    ::fetchRange(const QDateTime &from, const QDateTime &to) const
    {
        auto query = prepare(QString{"SELECT * FROM %1 WHERE timestamp BETWEEN ? AND ? ORDER BY timestamp ASC"}
            .arg(getTableName()));

        query.addBindValue(from);
        query.addBindValue(to);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    AssetValueSnapshotRepository::EntityList AssetValueSnapshotRepository
    ::fetchRange(Character::IdType characterId, const QDateTime &from, const QDateTime &to) const
    {
        auto query = prepare(QString{"SELECT * FROM %1 WHERE character_id = ? AND timestamp BETWEEN ? AND ? ORDER BY timestamp ASC"}
            .arg(getTableName()));

        query.addBindValue(characterId);
        query.addBindValue(from);
        query.addBindValue(to);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    QStringList AssetValueSnapshotRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "timestamp"
            << "character_id"
            << "balance";
    }

    void AssetValueSnapshotRepository::bindValues(const AssetValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != AssetValueSnapshot::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":timestamp", entity.getTimestamp());
        query.bindValue(":balance", entity.getBalance());
        query.bindValue(":character_id", entity.getCharacterId());
    }

    void AssetValueSnapshotRepository::bindPositionalValues(const AssetValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != AssetValueSnapshot::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getTimestamp());
        query.addBindValue(entity.getBalance());
        query.addBindValue(entity.getCharacterId());
    }
}
