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

#include "CorpMarketOrderValueSnapshotRepository.h"

namespace Evernus
{
    QString CorpMarketOrderValueSnapshotRepository::getTableName() const
    {
        return "corp_market_order_value_snapshots";
    }

    QString CorpMarketOrderValueSnapshotRepository::getIdColumn() const
    {
        return "id";
    }

    CorpMarketOrderValueSnapshotRepository::EntityPtr CorpMarketOrderValueSnapshotRepository::populate(const QSqlRecord &record) const
    {
        auto dt = record.value("timestamp").toDateTime();
        dt.setTimeSpec(Qt::UTC);

        auto corpMarketOrderValueSnapshot = std::make_shared<CorpMarketOrderValueSnapshot>(record.value("id").value<CorpMarketOrderValueSnapshot::IdType>());
        corpMarketOrderValueSnapshot->setTimestamp(dt);
        corpMarketOrderValueSnapshot->setCorporationId(record.value("corporation_id").toULongLong());
        corpMarketOrderValueSnapshot->setBuyValue(record.value("buy_value").toDouble());
        corpMarketOrderValueSnapshot->setSellValue(record.value("sell_value").toDouble());
        corpMarketOrderValueSnapshot->setNew(false);

        return corpMarketOrderValueSnapshot;
    }

    void CorpMarketOrderValueSnapshotRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY ASC,"
            "timestamp DATETIME NOT NULL,"
            "corporation_id BIGINT NOT NULL,"
            "buy_value DOUBLE NOT NULL,"
            "sell_value DOUBLE NOT NULL"
        ")"}.arg(getTableName()));

        exec(QString{"CREATE UNIQUE INDEX IF NOT EXISTS %1_corporation_timestamp ON %1(corporation_id, timestamp)"}.arg(getTableName()));
    }

    CorpMarketOrderValueSnapshotRepository::EntityList CorpMarketOrderValueSnapshotRepository
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

    QStringList CorpMarketOrderValueSnapshotRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "timestamp"
            << "corporation_id"
            << "buy_value"
            << "sell_value";
    }

    void CorpMarketOrderValueSnapshotRepository::bindValues(const CorpMarketOrderValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CorpMarketOrderValueSnapshot::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":timestamp", entity.getTimestamp());
        query.bindValue(":corporation_id", entity.getCorporationId());
        query.bindValue(":buy_value", entity.getBuyValue());
        query.bindValue(":sell_value", entity.getSellValue());
    }

    void CorpMarketOrderValueSnapshotRepository::bindPositionalValues(const CorpMarketOrderValueSnapshot &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CorpMarketOrderValueSnapshot::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getTimestamp());
        query.addBindValue(entity.getCorporationId());
        query.addBindValue(entity.getBuyValue());
        query.addBindValue(entity.getSellValue());
    }
}
