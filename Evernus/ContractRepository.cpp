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
#include <unordered_map>

#include <QSqlRecord>
#include <QSqlQuery>

#include "ContractItemRepository.h"
#include "ContractRepository.h"

namespace Evernus
{
    ContractRepository::ContractRepository(const ContractItemRepository &contractItemRepo,
                                           bool corp,
                                           const DatabaseConnectionProvider &connectionProvider)
        : Repository{connectionProvider}
        , mContractItemRepo{contractItemRepo}
        , mCorp{corp}
    {
    }

    QString ContractRepository::getTableName() const
    {
        return (mCorp) ? (QStringLiteral("corp_contracts")) : (QStringLiteral("contracts"));
    }

    QString ContractRepository::getIdColumn() const
    {
        return QStringLiteral("id");
    }

    ContractRepository::EntityPtr ContractRepository::populate(const QSqlRecord &record) const
    {
        auto issued = record.value(QStringLiteral("issued")).toDateTime();
        issued.setTimeSpec(Qt::UTC);

        auto expired = record.value(QStringLiteral("expired")).toDateTime();
        expired.setTimeSpec(Qt::UTC);

        auto accepted = record.value(QStringLiteral("accepted")).toDateTime();
        accepted.setTimeSpec(Qt::UTC);

        auto completed = record.value(QStringLiteral("completed")).toDateTime();
        completed.setTimeSpec(Qt::UTC);

        auto contract = std::make_shared<Contract>(record.value(QStringLiteral("id")).value<Contract::IdType>());
        contract->setIssuerId(record.value(QStringLiteral("issuer_id")).value<Character::IdType>());
        contract->setIssuerCorpId(record.value(QStringLiteral("issuer_corp_id")).toULongLong());
        contract->setAssigneeId(record.value(QStringLiteral("assignee_id")).toULongLong());
        contract->setAcceptorId(record.value(QStringLiteral("acceptor_id")).toULongLong());
        contract->setStartStationId(record.value(QStringLiteral("start_station_id")).toUInt());
        contract->setEndStationId(record.value(QStringLiteral("end_station_id")).toUInt());
        contract->setType(static_cast<Contract::Type>(record.value(QStringLiteral("type")).toInt()));
        contract->setStatus(static_cast<Contract::Status>(record.value(QStringLiteral("status")).toInt()));
        contract->setTitle(record.value(QStringLiteral("title")).toString());
        contract->setForCorp(record.value(QStringLiteral("for_corp")).toBool());
        contract->setAvailability(static_cast<Contract::Availability>(record.value(QStringLiteral("availability")).toInt()));
        contract->setIssued(issued);
        contract->setExpired(expired);
        contract->setAccepted(accepted);
        contract->setCompleted(completed);
        contract->setNumDays(record.value(QStringLiteral("num_days")).toInt());
        contract->setPrice(record.value(QStringLiteral("price")).toDouble());
        contract->setReward(record.value(QStringLiteral("reward")).toDouble());
        contract->setCollateral(record.value(QStringLiteral("collateral")).toDouble());
        contract->setBuyout(record.value(QStringLiteral("buyout")).toDouble());
        contract->setVolume(record.value(QStringLiteral("volume")).toDouble());
        contract->setNew(false);

        return contract;
    }

    void ContractRepository::create() const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "id BIGINT PRIMARY KEY,"
            "issuer_id BIGINT NOT NULL,"
            "issuer_corp_id BIGINT NOT NULL,"
            "assignee_id BIGINT NOT NULL,"
            "acceptor_id BIGINT NOT NULL,"
            "start_station_id INTEGER NOT NULL,"
            "end_station_id INTEGER NOT NULL,"
            "type TINYINT NOT NULL,"
            "status TINYINT NOT NULL,"
            "title TEXT NOT NULL,"
            "for_corp TINYINT NOT NULL,"
            "availability TINYINT NOT NULL,"
            "issued DATETIME NOT NULL,"
            "expired DATETIME NOT NULL,"
            "accepted DATETIME NULL,"
            "completed DATETIME NULL,"
            "num_days INTEGER NOT NULL,"
            "price DOUBLE NOT NULL,"
            "reward DOUBLE NOT NULL,"
            "collateral DOUBLE NOT NULL,"
            "buyout DOUBLE NOT NULL,"
            "volume DOUBLE NOT NULL"
        ")").arg(getTableName()));

        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_issuer ON %1(issuer_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_issuer_corp ON %1(issuer_corp_id)").arg(getTableName()));
        exec(QStringLiteral("CREATE INDEX IF NOT EXISTS %1_assignee ON %1(assignee_id)").arg(getTableName()));
    }

    ContractRepository::EntityList ContractRepository::fetchIssuedForCharacter(Character::IdType id) const
    {
        return fetchByColumnWithItems(id, QStringLiteral("issuer_id"));
    }

    ContractRepository::EntityList ContractRepository::fetchAssignedForCharacter(Character::IdType id) const
    {
        return fetchByColumnWithItems(id, QStringLiteral("assignee_id"));
    }

    ContractRepository::EntityList ContractRepository::fetchIssuedForCorporation(quint64 id) const
    {
        return fetchByColumnWithItems(id, QStringLiteral("issuer_corp_id"));
    }

    ContractRepository::EntityList ContractRepository::fetchAssignedForCorporation(quint64 id) const
    {
        return fetchByColumnWithItems(id, QStringLiteral("assignee_id"));
    }

    QStringList ContractRepository::getColumns() const
    {
        return QStringList{}
            << QStringLiteral("id")
            << QStringLiteral("issuer_id")
            << QStringLiteral("issuer_corp_id")
            << QStringLiteral("assignee_id")
            << QStringLiteral("acceptor_id")
            << QStringLiteral("start_station_id")
            << QStringLiteral("end_station_id")
            << QStringLiteral("type")
            << QStringLiteral("status")
            << QStringLiteral("title")
            << QStringLiteral("for_corp")
            << QStringLiteral("availability")
            << QStringLiteral("issued")
            << QStringLiteral("expired")
            << QStringLiteral("accepted")
            << QStringLiteral("completed")
            << QStringLiteral("num_days")
            << QStringLiteral("price")
            << QStringLiteral("reward")
            << QStringLiteral("collateral")
            << QStringLiteral("buyout")
            << QStringLiteral("volume");
    }

    void ContractRepository::bindValues(const Contract &entity, QSqlQuery &query) const
    {
        if (entity.getId() != Contract::invalidId)
            query.bindValue(QStringLiteral(":id"), entity.getId());

        query.bindValue(QStringLiteral(":issuer_id"), entity.getIssuerId());
        query.bindValue(QStringLiteral(":issuer_corp_id"), entity.getIssuerCorpId());
        query.bindValue(QStringLiteral(":assignee_id"), entity.getAssigneeId());
        query.bindValue(QStringLiteral(":acceptor_id"), entity.getAcceptorId());
        query.bindValue(QStringLiteral(":start_station_id"), entity.getStartStationId());
        query.bindValue(QStringLiteral(":end_station_id"), entity.getEndStationId());
        query.bindValue(QStringLiteral(":type"), static_cast<int>(entity.getType()));
        query.bindValue(QStringLiteral(":status"), static_cast<int>(entity.getStatus()));
        query.bindValue(QStringLiteral(":title"), entity.getTitle());
        query.bindValue(QStringLiteral(":for_corp"), entity.isForCorp());
        query.bindValue(QStringLiteral(":availability"), static_cast<int>(entity.getAvailability()));
        query.bindValue(QStringLiteral(":issued"), entity.getIssued());
        query.bindValue(QStringLiteral(":expired"), entity.getExpired());
        query.bindValue(QStringLiteral(":accepted"), entity.getAccepted());
        query.bindValue(QStringLiteral(":completed"), entity.getCompleted());
        query.bindValue(QStringLiteral(":num_days"), entity.getNumDays());
        query.bindValue(QStringLiteral(":price"), entity.getPrice());
        query.bindValue(QStringLiteral(":reward"), entity.getReward());
        query.bindValue(QStringLiteral(":collateral"), entity.getCollateral());
        query.bindValue(QStringLiteral(":buyout"), entity.getBuyout());
        query.bindValue(QStringLiteral(":volume"), entity.getVolume());
    }

    void ContractRepository::bindPositionalValues(const Contract &entity, QSqlQuery &query) const
    {
        if (entity.getId() != Contract::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getIssuerId());
        query.addBindValue(entity.getIssuerCorpId());
        query.addBindValue(entity.getAssigneeId());
        query.addBindValue(entity.getAcceptorId());
        query.addBindValue(entity.getStartStationId());
        query.addBindValue(entity.getEndStationId());
        query.addBindValue(static_cast<int>(entity.getType()));
        query.addBindValue(static_cast<int>(entity.getStatus()));
        query.addBindValue(entity.getTitle());
        query.addBindValue(entity.isForCorp());
        query.addBindValue(static_cast<int>(entity.getAvailability()));
        query.addBindValue(entity.getIssued());
        query.addBindValue(entity.getExpired());
        query.addBindValue(entity.getAccepted());
        query.addBindValue(entity.getCompleted());
        query.addBindValue(entity.getNumDays());
        query.addBindValue(entity.getPrice());
        query.addBindValue(entity.getReward());
        query.addBindValue(entity.getCollateral());
        query.addBindValue(entity.getBuyout());
        query.addBindValue(entity.getVolume());
    }

    void ContractRepository::preStore(Contract &entity) const
    {
        if (!entity.isNew())
            mContractItemRepo.deleteForContract(entity.getId());
    }

    template<class T>
    ContractRepository::EntityList ContractRepository::fetchByColumnWithItems(T id, const QString &column) const
    {
        // all this because https://bugreports.qt-project.org/browse/QTBUG-14904
        auto queryStr = QStringLiteral("SELECT %1.*, %4 FROM %1 LEFT JOIN %3 i ON i.contract_id = %1.id WHERE %1.%2 = ?")
            .arg(getTableName())
            .arg(column)
            .arg(mContractItemRepo.getTableName());

        auto columns = mContractItemRepo.getColumns();
        for (auto &column : columns)
            column = QString(QStringLiteral("i.%1 i_%1")).arg(column);

        queryStr = queryStr.arg(columns.join(QStringLiteral(", ")));

        auto query = prepare(queryStr);
        query.bindValue(0, id);

        DatabaseUtils::execQuery(query);

        EntityList result;

        const auto size = query.size();
        if (size > 0)
            result.reserve(size);

        std::unordered_map<Contract::IdType, EntityPtr> contracts;

        while (query.next())
        {
            const auto id = query.value(QStringLiteral("id")).template value<Contract::IdType>();
            auto it = contracts.find(id);
            if (it == std::end(contracts))
                it = contracts.emplace(id, populate(query.record())).first;

            if (!query.isNull(QStringLiteral("i_id")))
                it->second->addItem(mContractItemRepo.populate(QStringLiteral("i_"), query.record()));
        }

        for (auto &contract : contracts)
            result.emplace_back(std::move(contract.second));

        return result;
    }
}
