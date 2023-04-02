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

#include "FavoriteItemRepository.h"

namespace Evernus
{
    QString FavoriteItemRepository::getTableName() const
    {
        return "favorite_items";
    }

    QString FavoriteItemRepository::getIdColumn() const
    {
        return "id";
    }

    FavoriteItemRepository::EntityPtr FavoriteItemRepository::populate(const QSqlRecord &record) const
    {
        auto favoriteItem = std::make_shared<FavoriteItem>(record.value("id").value<FavoriteItem::IdType>());
        favoriteItem->setNew(false);

        return favoriteItem;
    }

    void FavoriteItemRepository::create() const
    {
        exec(QString{"CREATE TABLE IF NOT EXISTS %1 ("
            "id INTEGER PRIMARY KEY"
        ")"}.arg(getTableName()));
    }

    QStringList FavoriteItemRepository::getColumns() const
    {
        return QStringList{}
            << "id";
    }

    void FavoriteItemRepository::bindValues(const FavoriteItem &entity, QSqlQuery &query) const
    {
        if (entity.getId() != FavoriteItem::invalidId)
            query.bindValue(":id", entity.getId());
    }

    void FavoriteItemRepository::bindPositionalValues(const FavoriteItem &entity, QSqlQuery &query) const
    {
        if (entity.getId() != FavoriteItem::invalidId)
            query.addBindValue(entity.getId());
    }
}
