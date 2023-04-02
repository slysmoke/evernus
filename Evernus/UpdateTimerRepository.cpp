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

#include "UpdateTimerRepository.h"

namespace Evernus
{
    QString UpdateTimerRepository::getTableName() const
    {
        return "update_timers";
    }

    QString UpdateTimerRepository::getIdColumn() const
    {
        return "id";
    }

    UpdateTimerRepository::EntityPtr UpdateTimerRepository::populate(const QSqlRecord &record) const
    {
        auto dt = record.value("update_time").toDateTime();
        dt.setTimeSpec(Qt::UTC);

        auto updateTimer = std::make_shared<UpdateTimer>(record.value("id").value<UpdateTimer::IdType>());
        updateTimer->setCharacterId(record.value("character_id").value<Character::IdType>());
        updateTimer->setType(static_cast<TimerType>(record.value("type").toInt()));
        updateTimer->setUpdateTime(dt);
        updateTimer->setNew(false);

        return updateTimer;
    }

    void UpdateTimerRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY ASC,"
            "character_id BIGINT NOT NULL REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE,"
            "type TINYINT NOT NULL,"
            "update_time DATETIME NOT NULL,"

            "UNIQUE (character_id, type)"
        ")"}.arg(getTableName()).arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()));

        exec(QString{"CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)"}.arg(getTableName()).arg(characterRepo.getTableName()));
    }

    QStringList UpdateTimerRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "character_id"
            << "type"
            << "update_time";
    }

    void UpdateTimerRepository::bindValues(const UpdateTimer &entity, QSqlQuery &query) const
    {
        if (entity.getId() != UpdateTimer::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":character_id", entity.getCharacterId());
        query.bindValue(":type", static_cast<int>(entity.getType()));
        query.bindValue(":update_time", entity.getUpdateTime());
    }

    void UpdateTimerRepository::bindPositionalValues(const UpdateTimer &entity, QSqlQuery &query) const
    {
        if (entity.getId() != UpdateTimer::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(static_cast<int>(entity.getType()));
        query.addBindValue(entity.getUpdateTime());
    }
}
