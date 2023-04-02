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

#include "LMeveTaskRepository.h"

namespace Evernus
{
    QString LMeveTaskRepository::getTableName() const
    {
        return "lmeve_tasks";
    }

    QString LMeveTaskRepository::getIdColumn() const
    {
        return "id";
    }

    LMeveTaskRepository::EntityPtr LMeveTaskRepository::populate(const QSqlRecord &record) const
    {
        auto lMeveTask = std::make_shared<LMeveTask>(record.value("id").value<LMeveTask::IdType>());
        lMeveTask->setCharacterId(record.value("character_id").value<Character::IdType>());
        lMeveTask->setTypeId(record.value("type_id").value<EveType::IdType>());
        lMeveTask->setActivity(record.value("activity").toString());

        if (!record.isNull("runs"))
            lMeveTask->setRuns(record.value("runs").toUInt());
        if (!record.isNull("runs_done"))
            lMeveTask->setRunsDone(record.value("runs_done").toUInt());
        if (!record.isNull("runs_completed"))
            lMeveTask->setRunsCompleted(record.value("runs_completed").toUInt());
        if (!record.isNull("jobs_done"))
            lMeveTask->setJobsDone(record.value("jobs_done").toUInt());
        if (!record.isNull("jobs_success"))
            lMeveTask->setJobsSuccess(record.value("jobs_success").toUInt());
        if (!record.isNull("jobs_completed"))
            lMeveTask->setJobsCompleted(record.value("jobs_completed").toUInt());

        lMeveTask->setNew(false);

        return lMeveTask;
    }

    void LMeveTaskRepository::create(const Repository<Character> &characterRepo) const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "character_id BIGINT NOT NULL REFERENCES %2(%3) ON UPDATE CASCADE ON DELETE CASCADE,"
            "type_id INTEGER NOT NULL,"
            "activity TEXT NOT NULL,"
            "runs INTEGER NULL,"
            "runs_done INTEGER NULL,"
            "runs_completed INTEGER NULL,"
            "jobs_done INTEGER NULL,"
            "jobs_success INTEGER NULL,"
            "jobs_completed INTEGER NULL"
        ")"}.arg(getTableName()).arg(characterRepo.getTableName()).arg(characterRepo.getIdColumn()));

        exec(QString{"CREATE INDEX IF NOT EXISTS %1_%2_index ON %1(character_id)"}.arg(getTableName()).arg(characterRepo.getTableName()));
    }

    void LMeveTaskRepository::removeForCharacter(Character::IdType id) const
    {
        auto query = prepare(QString{"DELETE FROM %1 WHERE character_id = ?"}.arg(getTableName()));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);
    }

    LMeveTaskRepository::EntityList LMeveTaskRepository::fetchForCharacter(Character::IdType id) const
    {
        auto query = prepare(QString{"SELECT * FROM %1 WHERE character_id = ?"}.arg(getTableName()));
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        while (query.next())
            result.emplace_back(populate(query.record()));

        return result;
    }

    QStringList LMeveTaskRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "character_id"
            << "type_id"
            << "activity"
            << "runs"
            << "runs_done"
            << "runs_completed"
            << "jobs_done"
            << "jobs_success"
            << "jobs_completed";
    }

    void LMeveTaskRepository::bindValues(const LMeveTask &entity, QSqlQuery &query) const
    {
        if (entity.getId() != LMeveTask::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":character_id", entity.getCharacterId());
        query.bindValue(":type_id", entity.getTypeId());
        query.bindValue(":activity", entity.getActivity());
        query.bindValue(":runs", (entity.getRuns()) ? (*entity.getRuns()) : (QVariant{QVariant::UInt}));
        query.bindValue(":runs_done", (entity.getRunsDone()) ? (*entity.getRunsDone()) : (QVariant{QVariant::UInt}));
        query.bindValue(":runs_completed", (entity.getRunsCompleted()) ? (*entity.getRunsCompleted()) : (QVariant{QVariant::UInt}));
        query.bindValue(":jobs_done", (entity.getJobsDone()) ? (*entity.getJobsDone()) : (QVariant{QVariant::UInt}));
        query.bindValue(":jobs_success", (entity.getJobsSuccess()) ? (*entity.getJobsSuccess()) : (QVariant{QVariant::UInt}));
        query.bindValue(":jobs_completed", (entity.getJobsCompleted()) ? (*entity.getJobsCompleted()) : (QVariant{QVariant::UInt}));
    }

    void LMeveTaskRepository::bindPositionalValues(const LMeveTask &entity, QSqlQuery &query) const
    {
        if (entity.getId() != LMeveTask::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getCharacterId());
        query.addBindValue(entity.getTypeId());
        query.addBindValue(entity.getActivity());
        query.addBindValue((entity.getRuns()) ? (*entity.getRuns()) : (QVariant{QVariant::UInt}));
        query.addBindValue((entity.getRunsDone()) ? (*entity.getRunsDone()) : (QVariant{QVariant::UInt}));
        query.addBindValue((entity.getRunsCompleted()) ? (*entity.getRunsCompleted()) : (QVariant{QVariant::UInt}));
        query.addBindValue((entity.getJobsDone()) ? (*entity.getJobsDone()) : (QVariant{QVariant::UInt}));
        query.addBindValue((entity.getJobsSuccess()) ? (*entity.getJobsSuccess()) : (QVariant{QVariant::UInt}));
        query.addBindValue((entity.getJobsCompleted()) ? (*entity.getJobsCompleted()) : (QVariant{QVariant::UInt}));
    }
}
