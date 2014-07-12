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

#include "CacheTimerRepository.h"

namespace Evernus
{
    QString CacheTimerRepository::getTableName() const
    {
        return "cache_timers";
    }

    QString CacheTimerRepository::getIdColumn() const
    {
        return "id";
    }

    CacheTimer CacheTimerRepository::populate(const QSqlRecord &record) const
    {
        auto dt = record.value("cache_until").toDateTime();
        dt.setTimeSpec(Qt::UTC);

        CacheTimer cacheTimer{record.value("id").value<CacheTimer::IdType>(), dt};
        cacheTimer.setCharacterId(record.value("character_id").value<Character::IdType>());
        cacheTimer.setNew(false);

        return cacheTimer;
    }

    void CacheTimerRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QString{R"(CREATE TABLE IF NOT EXISTS %1 (
            id INTEGER PRIMARY KEY,
            character_id BIGINT NOT NULL REFERENCES %2(%3),
            cache_until DATETIME NOT NULL
        ))"}.arg(getTableName()).arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()));

        exec(QString{"CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)"}.arg(getTableName()).arg(characterRepo.getTableName()));
    }

    QStringList CacheTimerRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "character_id"
            << "cache_until";
    }

    void CacheTimerRepository::bindValues(const CacheTimer &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CacheTimer::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":character_id", entity.getCharacterId());
        query.bindValue(":cache_until", entity.getCacheUntil());
    }

    void CacheTimerRepository::bindPositionalValues(const CacheTimer &entity, QSqlQuery &query) const
    {
        if (entity.getId() != CacheTimer::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getCacheUntil());
    }
}