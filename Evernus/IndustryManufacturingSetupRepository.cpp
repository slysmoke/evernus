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
#include <QDataStream>

#include "IndustryManufacturingSetupRepository.h"

namespace Evernus
{
    QString IndustryManufacturingSetupRepository::getTableName() const
    {
        return QStringLiteral("industry_manufacturing_setups");
    }

    QString IndustryManufacturingSetupRepository::getIdColumn() const
    {
        return QStringLiteral("name");
    }

    IndustryManufacturingSetupRepository::EntityPtr IndustryManufacturingSetupRepository::populate(const QSqlRecord &record) const
    {
        auto setup = std::make_shared<IndustryManufacturingSetupEntity>(record.value(getIdColumn()).value<IndustryManufacturingSetupEntity::IdType>());
        setup->setSerializedSetup(record.value(QStringLiteral("data")).toByteArray());
        setup->setNew(false);

        return setup;
    }

    void IndustryManufacturingSetupRepository::create() const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "%2 TEXT PRIMARY KEY,"
            "data BLOB NOT NULL"
        ")").arg(getTableName()).arg(getIdColumn()));
    }

    QStringList IndustryManufacturingSetupRepository::getAllNames() const
    {
        auto query = exec(QString{"SELECT %1 FROM %2 ORDER BY %1"}.arg(getIdColumn()).arg(getTableName()));

        QStringList result;
        while (query.next())
            result << query.value(0).toString();

        return result;
    }

    QStringList IndustryManufacturingSetupRepository::getColumns() const
    {
        return {
            getIdColumn(),
            QStringLiteral("data"),
        };
    }

    void IndustryManufacturingSetupRepository::bindValues(const IndustryManufacturingSetupEntity &entity, QSqlQuery &query) const
    {
        if (entity.getId() != IndustryManufacturingSetupEntity::invalidId)
            query.bindValue(QStringLiteral(":") + getIdColumn(), entity.getId());

        query.bindValue(QStringLiteral(":data"), entity.getSerializedSetup());
    }

    void IndustryManufacturingSetupRepository::bindPositionalValues(const IndustryManufacturingSetupEntity &entity, QSqlQuery &query) const
    {
        if (entity.getId() != IndustryManufacturingSetupEntity::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getSerializedSetup());
    }
}
