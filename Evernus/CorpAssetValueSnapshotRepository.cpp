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

#include "CorpAssetValueSnapshotRepository.h"

namespace Evernus
{
    QString CorpAssetValueSnapshotRepository::getTableName() const
    {
        return "corp_asset_value_snapshots";
    }

    QString CorpAssetValueSnapshotRepository::getIdColumn() const
    {
        return "id";
    }

    CorpAssetValueSnapshotRepository::EntityPtr CorpAssetValueSnapshotRepository::populate(const QSqlRecord &record) const
    {
        auto dt = record.value("timestamp").toDateTime();
        dt.setTimeSpec(Qt::UTC);

        auto corpAssetValueSnapshot
            = std::make_shared<CorpAssetValueSnapshot>(record.value("id").value<CorpAssetValueSnapshot::IdType>(), record.value("balance").toDouble());
        corpAssetValueSnapshot->setTimestamp(dt);
        corpAssetValueSnapshot->setCorporationId(record.value("corporation_id").toULongLong());
        corpAssetValueSnapshot->setNew(false);

        return corpAssetValueSnapshot;
    }

    void CorpAssetValueSnapshotRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY ASC,"
            "timestamp DATETIME NOT NULL,"
            "corporation_id BIGINT NOT NULL,"
            "balance DOUBLE NOT NULL"
        ")"}.arg(getTableName()));

        exec(QString{"CREATE UNIQUE INDEX IF NOT EXISTS %1_corporation_timestamp ON %1(corporation_id, timestamp)"}.arg(getTableName()));
    }

    CorpAssetValueSnapshotRepository::EntityList CorpAssetValueSnapshotRepository
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

    CorpAssetValueSnapshotRepository::EntityList CorpAssetValueSnapshotRepository
    ::fetchRange(quint64 corporationId, const QDateTime &from, const QDateTime &to) const
    {
        auto query = prepare(QString{"SELECT * FROM %1 WHERE corporation_id = ? AND timestamp BETWEEN ? AND ? ORDER BY timestamp ASC"}
            .arg(getTableName()));

        query.addBindValue(corporationId);
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

    QStringList CorpAssetValueSnapshotRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "timestamp"
            << "corporation_id"
            << "balance";
    }

    void CorpAssetValueSnapshotRepository::bindValues(const CorpAssetValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CorpAssetValueSnapshot::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":timestamp", entity.getTimestamp());
        query.bindValue(":balance", entity.getBalance());
        query.bindValue(":corporation_id", entity.getCorporationId());
    }

    void CorpAssetValueSnapshotRepository::bindPositionalValues(const CorpAssetValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CorpAssetValueSnapshot::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getTimestamp());
        query.addBindValue(entity.getBalance());
        query.addBindValue(entity.getCorporationId());
    }
}
