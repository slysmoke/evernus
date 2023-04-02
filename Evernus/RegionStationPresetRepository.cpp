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
#include "DatabaseUtils.h"

#include "RegionStationPresetRepository.h"

namespace Evernus
{
    QString RegionStationPresetRepository::getTableName() const
    {
        return QStringLiteral("region_station_presets");
    }

    QString RegionStationPresetRepository::getIdColumn() const
    {
        return QStringLiteral("name");
    }

    RegionStationPresetRepository::EntityPtr RegionStationPresetRepository::populate(const QSqlRecord &record) const
    {
        auto preset = std::make_shared<RegionStationPreset>(record.value(getIdColumn()).value<RegionStationPreset::IdType>());
        preset->setSrcRegions(DatabaseUtils::decodeRawSet<RegionStationPreset::RegionSet::key_type>(record, QStringLiteral("src_regions")));
        preset->setDstRegions(DatabaseUtils::decodeRawSet<RegionStationPreset::RegionSet::key_type>(record, QStringLiteral("dst_regions")));
        preset->setNew(false);

        auto stationId = record.value(QStringLiteral("src_station_id"));
        if (!stationId.isNull())
            preset->setSrcStationId(stationId.value<RegionStationPreset::StationId::value_type>());

        stationId = record.value(QStringLiteral("dst_station_id"));
        if (!stationId.isNull())
            preset->setDstStationId(stationId.value<RegionStationPreset::StationId::value_type>());

        return preset;
    }

    void RegionStationPresetRepository::create() const
    {
        exec(QStringLiteral("CREATE TABLE IF NOT EXISTS %1 ("
            "%2 TEXT PRIMARY KEY,"
            "src_station_id BIGINT NULL,"
            "dst_station_id BIGINT NULL,"
            "src_regions BLOB NOT NULL,"
            "dst_regions BLOB NOT NULL"
        ")").arg(getTableName()).arg(getIdColumn()));
    }

    QStringList RegionStationPresetRepository::getAllNames() const
    {
        auto query = exec(QString{"SELECT %1 FROM %2 ORDER BY %1"}.arg(getIdColumn()).arg(getTableName()));

        QStringList result;
        while (query.next())
            result << query.value(0).toString();

        return result;
    }

    QStringList RegionStationPresetRepository::getColumns() const
    {
        return {
            getIdColumn(),
            QStringLiteral("src_station_id"),
            QStringLiteral("dst_station_id"),
            QStringLiteral("src_regions"),
            QStringLiteral("dst_regions")
        };
    }

    void RegionStationPresetRepository::bindValues(const RegionStationPreset &entity, QSqlQuery &query) const
    {
        if (entity.getId() != RegionStationPreset::invalidId)
            query.bindValue(QStringLiteral(":") + getIdColumn(), entity.getId());

        auto stationId = entity.getSrcStationId();
        query.bindValue(QStringLiteral(":src_station_id"), (stationId) ? (*stationId) : (QVariant{QVariant::ULongLong}));

        stationId = entity.getDstStationId();
        query.bindValue(QStringLiteral(":dst_station_id"), (stationId) ? (*stationId) : (QVariant{QVariant::ULongLong}));

        query.bindValue(QStringLiteral(":src_regions"), DatabaseUtils::encodeRawSet(entity.getSrcRegions()));
        query.bindValue(QStringLiteral(":dst_regions"), DatabaseUtils::encodeRawSet(entity.getDstRegions()));
    }

    void RegionStationPresetRepository::bindPositionalValues(const RegionStationPreset &entity, QSqlQuery &query) const
    {
        if (entity.getId() != RegionStationPreset::invalidId)
            query.addBindValue(entity.getId());

        auto stationId = entity.getSrcStationId();
        query.addBindValue((stationId) ? (*stationId) : (QVariant{QVariant::ULongLong}));

        stationId = entity.getDstStationId();
        query.addBindValue((stationId) ? (*stationId) : (QVariant{QVariant::ULongLong}));

        query.addBindValue(DatabaseUtils::encodeRawSet(entity.getSrcRegions()));
        query.addBindValue(DatabaseUtils::encodeRawSet(entity.getDstRegions()));
    }
}
