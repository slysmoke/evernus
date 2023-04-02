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

#include "LocationBookmarkRepository.h"

namespace Evernus
{
    QString LocationBookmarkRepository::getTableName() const
    {
        return "location_bookmarks";
    }

    QString LocationBookmarkRepository::getIdColumn() const
    {
        return "id";
    }

    LocationBookmarkRepository::EntityPtr LocationBookmarkRepository::populate(const QSqlRecord &record) const
    {
        auto locationBookmark = std::make_shared<LocationBookmark>(record.value("id").value<LocationBookmark::IdType>());
        locationBookmark->setRegionId(record.value("region_id").toUInt());
        locationBookmark->setSolarSystemId(record.value("solar_system_id").toUInt());
        locationBookmark->setStationId(record.value("station_id").toULongLong());
        locationBookmark->setNew(false);

        return locationBookmark;
    }

    void LocationBookmarkRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY,"
            "region_id INTEGER NOT NULL,"
            "solar_system_id INTEGER NOT NULL,"
            "station_id BIGINT NOT NULL,"

            "UNIQUE (region_id, solar_system_id, station_id)"
        ")"}.arg(getTableName()));
    }

    QStringList LocationBookmarkRepository::getColumns() const
    {
        return QStringList{}
            << "id"
            << "region_id"
            << "solar_system_id"
            << "station_id";
    }

    void LocationBookmarkRepository::bindValues(const LocationBookmark &entity, QSqlQuery &query) const
    {
        if (entity.getId() != LocationBookmark::invalidId)
            query.bindValue(":id", entity.getId());

        query.bindValue(":region_id", entity.getRegionId());
        query.bindValue(":solar_system_id", entity.getSolarSystemId());
        query.bindValue(":station_id", entity.getStationId());
    }

    void LocationBookmarkRepository::bindPositionalValues(const LocationBookmark &entity, QSqlQuery &query) const
    {
        if (entity.getId() != LocationBookmark::invalidId)
            query.addBindValue(entity.getId());

        query.addBindValue(entity.getRegionId());
        query.addBindValue(entity.getSolarSystemId());
        query.addBindValue(entity.getStationId());
    }
}
